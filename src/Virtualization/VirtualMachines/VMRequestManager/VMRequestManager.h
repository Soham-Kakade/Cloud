/*
 * VMRequestManager.h
 *
 *  Created on: Jun 1, 2016
 *      Author: zahra
 */

#ifndef VMREQUESTMANAGER_H_
#define VMREQUESTMANAGER_H_
#include "Request.h"
#include "icancloud_Base.h"
#include "RequestVM.h"
#include <vector>

class RequestVM;
class VMRequestManager: virtual public icancloud_Base{

protected:

    /** Bool that control if the main queue does not receive messages */
        bool schedulerQueueBlocked;

    /** If schedulerQueueBlocked = false, the requests are allocated in this queue */
        vector <AbstractRequest*> requestsQueue;

    /*************************************************************************************
      *                               Requests Management
      *************************************************************************************/
    /*
     * This method block the arrival of messages to the main queue
     */
    bool schedulerBlock();

    /*
     * This method unblock the temporal messages arrival..
     */
    bool schedulerUnblock();

     /*
      * To reinsert the request at the posiion index of the requests quetue

     void reinsertRequest (AbstractRequest* req, unsigned int index);
 */
     /*
      * Returns the set of requests from requestsQueue that id are the same as userModID
      */
     vector<AbstractRequest*> getRequestByUserID (int userModID);

     /*
      * Returns the request from requestsQueue at 'index' position
      */
     AbstractRequest* getRequestByIndex (unsigned int index);

     /*
      * Returns the size of the requests queue
      */
     int numPendingRequests();

     /*
      * Erase the request at position 'index'
      */
     void eraseRequest (unsigned int index);

     /*
      * Erase a request if compare(req, requestsQueue(i)) = true
      */
     void eraseRequest (AbstractRequest* req);

     /*****************************************************************
       *             Management of the temporal requests queue
       ****************************************************************/
      vector <AbstractRequest*> temporalRequestsQueue; /** If schedulerQueueBlocked = true, the requests are allocated in this queue */

      /*
       * This method enqueue the request at temporal requests queue if the arrival is blocked.
       *  If it is not blocked, the request will be enqueue at requests queue.
       */
      void user_request(AbstractRequest* request);

      /*
      * This method is responsible for block the requestsQueue redirecting the
      * incoming requests to the temporalQueue.
      */
         void blockArrivalRequests(bool blocked);

     /*****************************************************************
      *              Management of executing requests
      *****************************************************************/

     /*
      *  this vector allocates the pending requests that are been processed by node,
      *  such as the file storage or the fs creation.
      */
         vector<AbstractRequest*> executingRequests;

         /*
          * Insert an executing request into the vector
          */
         void insertExecutingRequest(AbstractRequest* req){executingRequests.push_back(req);};


         /*
          * Returns the executing requests vector size
          */
         int getExecutingRequestsSize(){return (int)executingRequests.size();};

         /*
          * Returns the executing request at position given as parameter
          */
         AbstractRequest* getExecutingRequest(int index){return (*(executingRequests.begin() + index));};

         /*
          * Erases the executing request at position given as parameter
          */
         void eraseExecutingRequest(int index){executingRequests.erase(executingRequests.begin() + index);};


public:
 //   RequestsManagement();

    VMRequestManager();
    virtual ~VMRequestManager();
    /*
     * Module initialization
     */
     virtual void initialize();

     /**
     * Module ending.
     */
     virtual void finish();
     void schedule();
     bool request_start_docker_container(RequestVM* req_vm);


    /*********************************************************************************
     *                                      Methods
     *********************************************************************************/

    /*
     * When an user realize an request, the manager insert it into the
     * request queue waiting until it will be processed.
     */
    void userSendRequest(AbstractRequest* request);
};

#endif /* VMREQUESTMANAGER_H_ */
