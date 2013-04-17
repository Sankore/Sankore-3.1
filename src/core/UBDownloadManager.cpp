/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBDownloadManager.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "gui/UBMainWindow.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/memcheck.h"


UBAsyncLocalFileDownloader::UBAsyncLocalFileDownloader(sDownloadFileDesc desc, QByteArray data, QObject *parent)
: QThread(parent)
, mDesc(desc)
, mData(data)
, m_bAborting(false)
{

}

UBAsyncLocalFileDownloader *UBAsyncLocalFileDownloader::download()
{
    start();
    return this;
}

void UBAsyncLocalFileDownloader::run()
{
    mMutex.lock();
    sDownloadFileDesc descriptor = mDesc;
    mMutex.unlock();

    descriptor.srcUrl = QUrl::fromUserInput(descriptor.srcUrl).toLocalFile();

    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(descriptor.srcUrl);

    int position=mimeType.indexOf(";");
    if(position != -1)
        mimeType=mimeType.left(position);

    UBMimeType::Enum itemMimeType = UBFileSystemUtils::mimeTypeFromString(mimeType);


    QString destDirectory;
    if (UBMimeType::Video == itemMimeType)
        destDirectory = UBPersistenceManager::videoDirectory;
    else
        if (UBMimeType::Audio == itemMimeType)
            destDirectory = UBPersistenceManager::audioDirectory;

    if (descriptor.originalSrcUrl.isEmpty())
        descriptor.originalSrcUrl = descriptor.srcUrl;

    QString uuid = QUuid::createUuid();
    UBPersistenceManager::persistenceManager()->addFileToDocument(UBApplication::boardController->selectedDocument(),
                                                                  descriptor.srcUrl,
                                                                  destDirectory,
                                                                  uuid,
                                                                  mTo,
                                                                  NULL);

    if (m_bAborting)
    {
        if (QFile::exists(mTo))
            QFile::remove(mTo);
    }
    else
        emit signal_asyncCopyFinished(descriptor.id, !mTo.isEmpty(), QUrl::fromLocalFile(mTo), QUrl(descriptor.originalSrcUrl), "", NULL, descriptor.pos, descriptor.size, descriptor.isBackground);
}

void UBAsyncLocalFileDownloader::abort()
{
    m_bAborting = true;
}

/** The unique instance of the download manager */
static UBDownloadManager* pInstance = NULL;

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBDownloadManager::UBDownloadManager(QObject *parent, const char *name):QObject(parent)
{
    setObjectName(name);
    init();

    connect(this, SIGNAL(fileAddedToDownload()), this, SLOT(onUpdateDownloadLists()));
}

/**
 * \brief Destructor
 */
UBDownloadManager::~UBDownloadManager()
{

}

/**
 * \brief Get the download manager
 * @return a pointer on the download manager
 */
UBDownloadManager* UBDownloadManager::downloadManager()
{
    if(NULL == pInstance)
    {
        pInstance = new UBDownloadManager();
    }
    return pInstance;
}

void UBDownloadManager::destroy()
{
    if(pInstance)
    {
        delete pInstance;
    }
    pInstance = NULL;
}

/**
 * \brief Add a file to the download list
 * @param desc as the given file description
 */
int UBDownloadManager::addFileToDownload(sDownloadFileDesc desc)
{
    // Set the ID for this download
    desc.id = mLastID;
    mLastID++;

    // Add the file to the pending download list
    mPendingDL.append(desc);

    // If the download is modal, show the download dialog
    if(desc.modal)
    {
        // Update the download order (priority to modal files)
        updateDownloadOrder();
        UBApplication::mainWindow->showDownloadWidget();
    }
    UBApplication::boardController->paletteManager()->startDownloads();

    emit fileAddedToDownload();

    return desc.id;
}

/**
 * \brief Initialize the download manager
 */
void UBDownloadManager::init()
{
    mCrntDL.clear();
    mPendingDL.clear();
    mDownloads.clear();
    mLastID = 1;
    mDLAvailability.clear();
    for(int i=0; i<SIMULTANEOUS_DOWNLOAD; i++)
    {
        mDLAvailability.append(-1);
    }
}

/**
 * \brief Update the download order. The modal downloads will be put in priority.
 */
void UBDownloadManager::updateDownloadOrder()
{
    QVector<sDownloadFileDesc> modalFiles;
    QVector<sDownloadFileDesc> nonModalfiles;

    for(int i=0; i<mPendingDL.size(); i++)
    {
        sDownloadFileDesc crnt = mPendingDL.at(i);
        if(crnt.modal)
        {
            modalFiles.append(crnt);
        }
        else
        {
            nonModalfiles.append(crnt);
        }
    }

    mPendingDL = modalFiles + nonModalfiles;
}

/**
 * \brief Update the download list. If a current download is finished, we take a
 *  file from the pending download list and add it to the download list.
 */
void UBDownloadManager::onUpdateDownloadLists()
{
    for(int i=0; i<SIMULTANEOUS_DOWNLOAD; i++)
    {
        if(mPendingDL.empty())
        {
            // If we fall here that means that there is no pending download
            break;
        }
        if(-1 == mDLAvailability.at(i))        {
            // Pending downloads exist and a download 'slot' is available
            // Let's move the first pending download to the current download
            // list and fill the slot
            sDownloadFileDesc desc = mPendingDL.at(0);
            mCrntDL.append(desc);
            mPendingDL.remove(0);
            mDLAvailability.remove(i);
            mDLAvailability.insert(i, desc.id);

            // Start the download of this file
            startFileDownload(desc);
        }
    }
}

/**
 * \brief Get the list of the current downloads
 * @return a QVector of current downloads
 */
QVector<sDownloadFileDesc> UBDownloadManager::currentDownloads()
{
    return mCrntDL;
}

/**
 * \brief Get the list of the pending downloads
 * @return a QVector of pending downloads
 */
QVector<sDownloadFileDesc> UBDownloadManager::pendingDownloads()
{
    return mPendingDL;
}

/**
 * \brief Update the file transfer information
 * @param desc as the current downloaded file description
 */
void UBDownloadManager::onDownloadProgress(int id, qint64 received, qint64 total)
{
    updateFileCurrentSize(id, received, total);
}

/**
 * \brief Called when the download of the given file is finished
 * @param desc as the current downloaded file description
 */

void UBDownloadManager::onDownloadFinished(int id, bool pSuccess, QUrl sourceUrl, QUrl contentUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground)
{
//    Temporary data for dnd do not delete it please
    Q_UNUSED(pPos)
    Q_UNUSED(pSize)
    Q_UNUSED(isBackground)

    for(int i=0; i<mCrntDL.size(); i++)
    {
        sDownloadFileDesc desc = mCrntDL.at(i);
        if(id == desc.id)
        {
            if (desc.dest == sDownloadFileDesc::graphicsWidget) {
                desc.contentTypeHeader = pContentTypeHeader;
                emit downloadFinished(pSuccess, desc, pData);

            }
            else if(desc.dest == sDownloadFileDesc::board) {
                // The downloaded file is modal so we must put it on the board
                emit addDownloadedFileToBoard(pSuccess, sourceUrl, contentUrl, pContentTypeHeader, pData, pPos, pSize, false, isBackground);
            }
            else if(desc.dest == sDownloadFileDesc::library)
            {
                emit addDownloadedFileToLibrary(pSuccess, sourceUrl, pContentTypeHeader, pData, desc.name);
            }

            break;
        }
    }

    // Then do this
    updateFileCurrentSize(id);
}

/**
 * \brief Update the description of the given current downloaded file
 * @param desc as the current downloaded file description
 */
void UBDownloadManager::updateFileCurrentSize(int id, qint64 received, qint64 total)
{
    for(int i=0; i<mCrntDL.size();i++)
    {
        if(mCrntDL.at(i).id == id)
        {
            sDownloadFileDesc desc = mCrntDL.at(i);
            if(received >= 0 && total >= 0)
            {
                // -------------------------------------
                // [=============== x% ====            ]
                // -------------------------------------
                desc.currentSize = received;
                desc.totalSize = total;

                emit downloadUpdated(id, received, total);
            }
            else
            {
                // -------------------------------------
                // [=============== 100% ==============]
                // -------------------------------------
                // received and total are negative. That means that the download is finished
                desc.currentSize = mCrntDL.at(i).totalSize;

                // Remove the finished file from the current download list
                mCrntDL.remove(i);

                // Here we don't forget to remove the reply related to the finished download
                mDownloads.remove(id);

                // Free the download slot used by the finished file
                for(int j=0; j<mDLAvailability.size();j++)
                {
                    if(id == mDLAvailability.at(j))
                    {
                        mDLAvailability.remove(j);
                        mDLAvailability.insert(j, -1);
                        break;
                    }
                }

                // Here we check if some modal downloads remain
                checkIfModalRemains();

                // Then we update the list of downloads
                onUpdateDownloadLists();

                emit downloadFinished(id);

                // Verify if all downloads are finished
                if(mCrntDL.empty() && mPendingDL.empty())
                {
                    finishDownloads();
                }

                break;
            }

            mCrntDL.remove(i);
            mCrntDL.insert(i,desc);
            break;
        }
    }
}

/**
 * \brief Start the download of a file
 * @param desc as the given file description
 */
void UBDownloadManager::startFileDownload(sDownloadFileDesc desc)
{
    if (desc.srcUrl.startsWith("file://") || desc.srcUrl.startsWith("/"))
    {
        UBAsyncLocalFileDownloader * cpHelper = new UBAsyncLocalFileDownloader(desc, QByteArray(), this);
        connect(cpHelper, SIGNAL(signal_asyncCopyFinished(int, bool, QUrl, QUrl, QString, QByteArray, QPointF, QSize, bool)), this, SLOT(onDownloadFinished(int, bool, QUrl, QUrl,QString, QByteArray, QPointF, QSize, bool)));
        QObject *res = dynamic_cast<QObject *>(cpHelper->download());
        if (!res)
            delete res;
        else
            mDownloads[desc.id] = res;
    }
    else
    {
        UBDownloadHttpFile* http = new UBDownloadHttpFile(desc.id, this);
        connect(http, SIGNAL(downloadProgress(int, qint64,qint64)), this, SLOT(onDownloadProgress(int,qint64,qint64)));
        connect(http, SIGNAL(downloadFinished(int, bool, QUrl, QUrl, QString, QByteArray, QPointF, QSize, bool)), this, SLOT(onDownloadFinished(int, bool, QUrl, QUrl, QString, QByteArray, QPointF, QSize, bool)));
        connect(http, SIGNAL(downloadError(int)), this, SLOT(onDownloadError(int)));

        //the desc.srcUrl is encoded. So we have to decode it before.
        QUrl url;
        url.setEncodedUrl(desc.srcUrl.toUtf8());
        // We send here the request and store its reply in order to be able to cancel it if needed
        mDownloads[desc.id] = dynamic_cast<QObject *>(http->get(url, desc.pos, desc.size, desc.isBackground));
    }
}

/**
 * \brief Verify if modal downloads remains and notify everyone if it is not the case.
 */
void UBDownloadManager::checkIfModalRemains()
{
    bool bModal = false;
    for(int i=0; i<mCrntDL.size();i++)
    {
        if(mCrntDL.at(i).modal)
        {
            bModal = true;
            break;
        }
    }

    if(!bModal)
    {
        for(int j=0; j<mPendingDL.size(); j++)
        {
            if(mPendingDL.at(j).modal)
            {
                bModal = true;
                break;
            }
        }
    }

    if(bModal || (mCrntDL.empty() && mPendingDL.empty()))
    {
        // Close the modal window
        UBApplication::mainWindow->hideDownloadWidget();

        // Notify that no modal downloads are pending
        emit downloadModalFinished();
    }
}

/**
 * \brief Cancel all downloads
 */
void UBDownloadManager::cancelDownloads()
{
    // Stop the current downloads
    QMap<int, QObject*>::iterator it = mDownloads.begin();
    for(; it!=mDownloads.end();it++)
    {
        QNetworkReply *netReply = dynamic_cast<QNetworkReply*>(it.value());
        if (netReply)
            netReply->abort();
        else
        {
            UBAsyncLocalFileDownloader *localDownload = dynamic_cast<UBAsyncLocalFileDownloader *>(it.value());
            if (localDownload)
                localDownload->abort();
        }
    }

    // Clear all the lists
    init();

    checkIfModalRemains();

    finishDownloads(true);
}

void UBDownloadManager::onDownloadError(int id)
{
    QNetworkReply *pReply = dynamic_cast<QNetworkReply *>(mDownloads.value(id));

    if(NULL != pReply)
    {
        QString errorString = tr("Download failed.");
        // Check which error occured:
        switch(pReply->error())
        {
            case QNetworkReply::ConnectionRefusedError:
                errorString += " " + (tr("the remote server refused the connection (the server is not accepting requests)"));
                break;
            case QNetworkReply::RemoteHostClosedError:
                errorString += " " + (tr("the remote server closed the connection prematurely, before the entire reply was received and processed"));
                break;
            case QNetworkReply::HostNotFoundError:
                errorString += " " + (tr("the remote host name was not found (invalid hostname)"));
                break;
            case QNetworkReply::TimeoutError:
                errorString += " " + (tr("the connection to the remote server timed out"));
                break;
            case QNetworkReply::OperationCanceledError:
                errorString += " " + (tr("the operation was canceled via calls to abort() or close() before it was finished."));
                break;
            case QNetworkReply::SslHandshakeFailedError:
                errorString += " " + (tr("the SSL/TLS handshake failed and the encrypted channel could not be established. The sslErrors() signal should have been emitted."));
                break;
            case QNetworkReply::TemporaryNetworkFailureError:
                errorString += " " + (tr("the connection was broken due to disconnection from the network, however the system has initiated roaming to another access point. The request should be resubmitted and will be processed as soon as the connection is re-established."));
                break;
            case QNetworkReply::ProxyConnectionRefusedError:
                errorString += " " + (tr("the connection to the proxy server was refused (the proxy server is not accepting requests)"));
                break;
            case QNetworkReply::ProxyConnectionClosedError:
                errorString += " " + (tr("the proxy server closed the connection prematurely, before the entire reply was received and processed"));
                break;
            case QNetworkReply::ProxyNotFoundError:
                errorString += " " + (tr("the proxy host name was not found (invalid proxy hostname)"));
                break;
            case QNetworkReply::ProxyTimeoutError:
                errorString += " " + (tr("the connection to the proxy timed out or the proxy did not reply in time to the request sent"));
                break;
            case QNetworkReply::ProxyAuthenticationRequiredError:
                errorString += " " + (tr("the proxy requires authentication in order to honour the request but did not accept any credentials offered (if any)"));
                break;
            case QNetworkReply::ContentAccessDenied:
                errorString += " " + (tr("the access to the remote content was denied (similar to HTTP error 401)"));
                break;
            case QNetworkReply::ContentOperationNotPermittedError:
                errorString += " " + (tr("the operation requested on the remote content is not permitted"));
                break;
            case QNetworkReply::ContentNotFoundError:
                errorString += " " + (tr("the remote content was not found at the server (similar to HTTP error 404)"));
                break;
            case QNetworkReply::AuthenticationRequiredError:
                errorString += " " + (tr("the remote server requires authentication to serve the content but the credentials provided were not accepted (if any)"));
                break;
            case QNetworkReply::ContentReSendError:
                errorString += " " + (tr("the request needed to be sent again, but this failed for example because the upload data could not be read a second time."));
                break;
            case QNetworkReply::ProtocolUnknownError:
                errorString += " " + (tr("the Network Access API cannot honor the request because the protocol is not known"));
                break;
            case QNetworkReply::ProtocolInvalidOperationError:
                errorString += " " + (tr("the requested operation is invalid for this protocol"));
                break;
            case QNetworkReply::UnknownNetworkError:
                errorString += " " + (tr("an unknown network-related error was detected"));
                break;
            case QNetworkReply::UnknownProxyError:
                errorString += " " + (tr("an unknown proxy-related error was detected"));
                break;
            case QNetworkReply::UnknownContentError:
                errorString += " " + (tr("an unknown error related to the remote content was detected"));
                break;
            case QNetworkReply::ProtocolFailure:
                errorString += " " + (tr("a breakdown in protocol was detected (parsing error, invalid or unexpected responses, etc.)"));
                break;
            default:
                // Check the documentation of QNetworkReply in Qt Assistant for the different error cases
            break;
        }
        UBApplication::showMessage(errorString);
        cancelDownload(id);
    }
}

void UBDownloadManager::finishDownloads(bool cancel)
{
    UBApplication::boardController->paletteManager()->stopDownloads();
    if(cancel){
        emit cancelAllDownloads();
    }
    else{
        emit allDownloadsFinished();
    }
}

void UBDownloadManager::cancelDownload(int id)
{
    if (!mDownloads.size())
        return;

    // Stop the download

    QNetworkReply *pNetworkDownload = dynamic_cast<QNetworkReply *>(mDownloads[id]);
    if (pNetworkDownload)
        pNetworkDownload->abort();
    else
    {
        UBAsyncLocalFileDownloader *pLocalDownload = dynamic_cast<UBAsyncLocalFileDownloader *>(mDownloads[id]);
        if (pLocalDownload)
        {
            if (pLocalDownload->isRunning())
                pLocalDownload->abort();
        }
    }

    mDownloads.remove(id);

    // Remove the canceled download from the download lists
    bool bFound = false;
    for(int i=0; i<mCrntDL.size(); i++){
        if(id == mCrntDL.at(i).id){
            mCrntDL.remove(i);
            bFound = true;
            break;
        }
    }
    if(!bFound){
        for(int j=0; j<mPendingDL.size(); j++){
            if(id == mPendingDL.at(j).id){
                mPendingDL.remove(j);
                bFound = true;
                break;
            }
        }
    }

    // Free the download slot used by the finished file
    for(int h=0; h<mDLAvailability.size();h++){
        if(id == mDLAvailability.at(h)){
            mDLAvailability.remove(h);
            mDLAvailability.insert(h, -1);
            break;
        }
    }

    // Here we check if some modal downloads remain
    checkIfModalRemains();

    // Then we update the list of downloads
    onUpdateDownloadLists();

    // Verify if all downloads are finished
    if(mCrntDL.empty() && mPendingDL.empty())
    {
        finishDownloads();
    }
}

// ------------------------------------------------------------------------------
/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBDownloadHttpFile::UBDownloadHttpFile(int fileId, QObject *parent):UBHttpGet(parent)
{
    mId = fileId;

    connect(this, SIGNAL(downloadFinished(bool,QUrl,QString,QByteArray,QPointF,QSize,bool)), this, SLOT(onDownloadFinished(bool,QUrl,QString,QByteArray,QPointF,QSize,bool)));
    connect(this, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
}

/**
 * \brief Destructor
 */
UBDownloadHttpFile::~UBDownloadHttpFile()
{

}

/**
 * \brief Handles the download progress notification
 * @param bytesReceived as the number of received bytes
 * @param bytesTotal as the total number of bytes
 */
void UBDownloadHttpFile::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(mId, bytesReceived, bytesTotal);
}

/**
 * \brief Handles the download finished notification
 * @param pSuccess as the success indicator
 * @param sourceUrl as the source URL
 * @param pContentTypeHeader as the response content type header
 * @param pData as the packet data
 * @param pPos as the item position in the board
 * @param psize as the item size (GUI)
 * @param isBackground as the background mdoe indicator
 */
void UBDownloadHttpFile::onDownloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground)
{
    if(pSuccess)
    {
        // Notify the end of the download
        emit downloadFinished(mId, pSuccess, sourceUrl, sourceUrl, pContentTypeHeader, pData, pPos, pSize, isBackground);
    }
    else
    {
        // Notify the fact that and error occured during the download
        emit downloadError(mId);
    }
}

