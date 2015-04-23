﻿#include "FileQueueUploaderPrivate.h"

#include <thread>
#include <algorithm>
#include "DefaultUploadEngine.h"
#include "FileQueueUploader.h"
#include "Uploader.h"
#include "Core/Upload/FileUploadTask.h"
#include "Gui/Dialogs/LogWindow.h"
#include "UploadEngineManager.h"
#include "Core/Upload/UploadFilter.h"
#include <zthread/ZThread.h>

/* private CFileQueueUploaderPrivate class */
class FileQueueUploaderPrivate::Runnable
#ifndef IU_CLI
	: public ZThread::Runnable
#endif

{
public:
	FileQueueUploaderPrivate* uploader_;
	Runnable(FileQueueUploaderPrivate* uploader)
	{
		uploader_ = uploader;
	}

	virtual void run()
	{
		uploader_->run();
	}
};

TaskAcceptorBase::TaskAcceptorBase(bool useMutex )
{
	fileCount = 0;
	useMutex_ = useMutex;
}

bool TaskAcceptorBase::canAcceptUploadTask(UploadTask* task)
{
	if (useMutex_) {
		serverThreadsMutex_.lock();
	}
	std::string serverName = task->serverProfile().serverName();
	auto it = serverThreads_.find(serverName);
	if (it == serverThreads_.end())
	{
		ServerThreadsInfo sti;
		sti.ued = task->serverProfile().uploadEngineData();
		sti.runningThreads = 1;
		serverThreads_[serverName] = sti;
		fileCount++;
		if (useMutex_) {
			serverThreadsMutex_.unlock();
		}
		return true;
	}
	if (!it->second.ued )
	{
		it->second.ued = task->serverProfile().uploadEngineData(); // FIXME
	}

	if ((!it->second.ued->MaxThreads || it->second.runningThreads < it->second.ued->MaxThreads) && !it->second.fatalError)
	{
		it->second.runningThreads++;
		fileCount++;
		if (useMutex_) {
			serverThreadsMutex_.unlock();
		}
		return true;
	}
	if (useMutex_) {
		serverThreadsMutex_.unlock();
	}
	return false;
}

FileQueueUploaderPrivate::FileQueueUploaderPrivate(CFileQueueUploader* queueUploader, UploadEngineManager* uploadEngineManager) {
	m_nThreadCount = 3;
	m_NeedStop = false;
	m_IsRunning = false;
	m_nRunningThreads = 0;
	queueUploader_ = queueUploader;
	startFromSession_ = 0;
	uploadEngineManager_ = uploadEngineManager;
	autoStart_ = true;
}

FileQueueUploaderPrivate::~FileQueueUploaderPrivate() {
}

bool FileQueueUploaderPrivate::onNeedStopHandler() {
	return m_NeedStop;
}

void FileQueueUploaderPrivate::onProgress(CUploader* uploader, InfoProgress progress) {
	auto task = uploader->currentTask();
	UploadProgress* prog = task->progress();
	prog->totalUpload = progress.Total;
	prog->uploaded = progress.Uploaded;
	prog->isUploading = progress.IsUploading;
	
	if (task->OnUploadProgress) {
		task->OnUploadProgress(task.get());
	}
}

void FileQueueUploaderPrivate::onErrorMessage(CUploader*, ErrorInfo ei)
{
	DefaultErrorHandling::ErrorMessage(ei);
}

void FileQueueUploaderPrivate::onDebugMessage(CUploader*, const std::string& msg, bool isResponseBody)
{
	DefaultErrorHandling::DebugMessage(msg, isResponseBody);
}

void FileQueueUploaderPrivate::onTaskAdded(UploadSession*, UploadTask* task)
{
	sessionsMutex_.lock();
	/*FileUploadTask* fut = dynamic_cast<FileUploadTask*>(task);
	if ( fut )
	{
		FileUploadTask* parent = dynamic_cast<FileUploadTask*>(fut->parentTask());
		LOG(ERROR) << "FileQueueUploaderPrivate::onTaskAdded " << fut->getFileName() << (parent ? "\r\nparent="+parent->getDisplayName() : "");
		
	}*/
	startFromSession_ = 0;
	sessionsMutex_.unlock();
	start();
}

int FileQueueUploaderPrivate::pendingTasksCount()
{
	std::lock_guard<std::mutex> lock(serverThreadsMutex_);
	std::lock_guard<std::mutex> lock2(sessionsMutex_);
	TaskAcceptorBase acceptor(false); // do not use mutex
	acceptor.serverThreads_ = this->serverThreads_;
	for (size_t i = startFromSession_; i < sessions_.size(); i++)
	{
		sessions_[i]->pendingTasksCount(&acceptor);
	}
	return acceptor.fileCount;
}

void FileQueueUploaderPrivate::taskAdded(UploadTask* task)
{
	queueUploader_->taskAdded(task);
}

void FileQueueUploaderPrivate::OnConfigureNetworkClient(CUploader* uploader, NetworkClient* nm)
{
	if (  queueUploader_->OnConfigureNetworkClient )
	{
		queueUploader_->OnConfigureNetworkClient(queueUploader_, nm);
	}
	/*if (callback_) {
		callback_->OnConfigureNetworkClient(queueUploader_, nm);
	}*/
}

std_tr::shared_ptr<UploadTask> FileQueueUploaderPrivate::getNextJob() {
	if (m_NeedStop)
		return std_tr::shared_ptr<UploadTask>();
#ifndef IU_CLI
	std::lock_guard<std::mutex> lock(mutex_);
#endif
	//LOG(INFO) << "startFromSession_=" << startFromSession_;
	if (!sessions_.empty() && !m_NeedStop)
	{
		for (size_t i = startFromSession_; i < sessions_.size(); i++)
		{
			std_tr::shared_ptr<UploadTask> task;
			if (!sessions_[i]->getNextTask(this, task))
			{
				startFromSession_ = i + 1;
			}
			if (task) {
				task->setRunning(true);
				task->setFinished(false);

				return task;
			}
			
		}
	}
#ifndef IU_CLI

#endif
	return std_tr::shared_ptr<UploadTask>();
}

void FileQueueUploaderPrivate::AddTask(std_tr::shared_ptr<UploadTask>  task) {
	std::shared_ptr<UploadSession> session(new UploadSession());
	session->addTask(task);
	AddSession(session);
	//serverThreads_[task->serv].waitingFileCount++;
	//AddFile(newTask);
}

void FileQueueUploaderPrivate::AddSession(std::shared_ptr<UploadSession> uploadSession)
{
	sessionsMutex_.lock();
	uploadSession->addTaskAddedCallback(UploadSession::TaskAddedCallback(this, &FileQueueUploaderPrivate::onTaskAdded));
	int count = uploadSession->taskCount();
	for (int i = 0; i < count; i++ )
	{
		taskAdded(uploadSession->getTask(i).get());
	}
	sessions_.push_back(uploadSession);
	sessionsMutex_.unlock();
	if (autoStart_)
	{
		start();
	}
}

void FileQueueUploaderPrivate::removeSession(std::shared_ptr<UploadSession> uploadSession)
{
	std::lock_guard<std::mutex> lock(sessionsMutex_);
	auto it = std::find(sessions_.begin(), sessions_.end(), uploadSession);
	if (it != sessions_.end() )
	{
		sessions_.erase(it);
	}
	startFromSession_ = 0;
}

void FileQueueUploaderPrivate::addUploadFilter(UploadFilter* filter)
{
	filters_.push_back(filter);
}

void FileQueueUploaderPrivate::removeUploadFilter(UploadFilter* filter)
{
	auto it = std::find(filters_.begin(), filters_.end(), filter);
	if (it != filters_.end())
	{
		filters_.erase(it);
	}
}

void FileQueueUploaderPrivate::start() {
	std::lock_guard<std::mutex> lock(mutex_);
	m_NeedStop = false;
	m_IsRunning = true;
	int numThreads = std::min<int>(size_t(m_nThreadCount - m_nRunningThreads), pendingTasksCount());

	for (int i = 0; i < numThreads; i++)
	{
		m_nRunningThreads++;
		ZThread::Thread t1(new Runnable(this));// starting new thread

		/*std::thread t1(&FileQueueUploaderPrivate::run, this);
		t1.detach();*/

	}
}


void FileQueueUploaderPrivate::run()
{
	CUploader uploader;
	uploader.onConfigureNetworkClient.bind(this, &FileQueueUploaderPrivate::OnConfigureNetworkClient);
	
	
	// TODO
	uploader.onErrorMessage.bind(this, &FileQueueUploaderPrivate::onErrorMessage);
	uploader.onDebugMessage.bind(this, &FileQueueUploaderPrivate::onDebugMessage);
	
	for (;;)
	{
		auto it = getNextJob();
		FileUploadTask* fut = dynamic_cast<FileUploadTask*>(it.get());
		//LOG(ERROR) << "getNextJob() returned " << (fut ? fut->getFileName() : "NULL");
		if (!it)
			break;
		
		mutex_.lock();
		serverThreads_[it->serverName()].waitingFileCount--;
		std::string serverName = it->serverName();
		//serverThreads_[serverName].runningThreads++;
		mutex_.unlock();

		//uploader.onProgress.bind(this, &FileQueueUploaderPrivate::onProgress);

		for (int i = 0; i < filters_.size(); i++) {
			filters_[i]->PreUpload(it.get()); // ServerProfile can be changed in PreUpload filters
		}
		CAbstractUploadEngine *engine = uploadEngineManager_->getUploadEngine(it->serverProfile());
		if (!engine)
		{
			it->setFinished(true);
			continue;
		}
		
		uploader.setUploadEngine(engine);
		uploader.onNeedStop.bind(this, &FileQueueUploaderPrivate::onNeedStopHandler);

		//LOG(ERROR) << "uploader.Upload(it) " << (fut ? fut->getFileName() : "NULL");
		bool res = uploader.Upload(it);
		//LOG(ERROR) << "uploader.Upload(it) finished " << (fut ? fut->getFileName() : "NULL");
		
#ifndef IU_CLI
		serverThreadsMutex_.lock();
#endif
		if (!res && uploader.isFatalError())
		{
			serverThreads_[serverName].fatalError = true;
		}
		serverThreads_[serverName].runningThreads--;
#ifndef IU_CLI
		serverThreadsMutex_.unlock();
#endif

		// m_CS.Lock();
#ifndef IU_CLI
		callMutex_.lock();
#endif
		UploadResult* result = it->uploadResult();
		result->serverName = serverName;

		if (res) {
			result->directUrl = (uploader.getDirectUrl());
			result->downloadUrl = (uploader.getDownloadUrl());
			if (result->thumbUrl.empty()) {
				result->thumbUrl = (uploader.getThumbUrl());
			}
			it->setUploadSuccess(true);
			for (int i = 0; i < filters_.size(); i++) {
				filters_[i]->PostUpload(it.get());
			}
		}
		else
		{
			it->setUploadSuccess(false);
		}
		it->setFinished(true);
		it->setRunning(false);
#ifndef IU_CLI
		callMutex_.unlock();
#endif
	}
#ifndef IU_CLI
	mutex_.lock();
#endif
	m_nRunningThreads--;
#ifndef IU_CLI
	mutex_.unlock();
#endif
	uploadEngineManager_->clearThreadData();
	if (!m_nRunningThreads)
	{
		m_IsRunning = false;
		if (queueUploader_->OnQueueFinished) {
			queueUploader_->OnQueueFinished(queueUploader_);
		}
		//LOG(ERROR) << "All threads terminated";
	}

}
