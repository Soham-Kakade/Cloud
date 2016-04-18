//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "CloudSchedulerRR.h"

Define_Module(CloudSchedulerRR);

//-------------------Scheduling metods.------------------------------------

ICCLog csrr_f;

void CloudSchedulerRR::initialize(){

    AbstractCloudManager::initialize();
    minimum_percent_storageNode_free = 0.0;
    printNodePowerConsumption = false;
    printNodeEnergyConsumed =  false;
    printComponentsPowerConsumption = false;
    printComponentsEnergyConsumed = false;
    printDataCenterPowerConsumption = false;
    printDataCenterEnergyConsumed = false;

    dc_EnergyConsumed = 0.0;
    maximum_number_of_processes_per_node = par("numberOfVMperNode");
    currentNodeIndex = 0;
    currentNodeType = 0;
}

//-------------------Scheduling metods.------------------------------------

void CloudSchedulerRR::setupScheduler(){

    minimum_percent_storageNode_free = par("minimum_percent_storageNode_free").doubleValue();
    printNodePowerConsumption = par("printNodePowerConsumed").boolValue();
    printNodeEnergyConsumed = par("printNodeEnergyConsumed").boolValue();
    printComponentsPowerConsumption = par("printComponentsPowerConsumed").boolValue();
    printComponentsEnergyConsumed = par("printComponentsEnergyConsumed").boolValue();
    printDataCenterPowerConsumption = par("printDataCenterPowerConsumed").boolValue();
    printDataCenterEnergyConsumed = par("printDataCenterEnergyConsumed").boolValue();

    dc_EnergyConsumed = 0.0;

    AllocationManagement::setupStorageNodes();

    // reconfigure if it will be needed..
    ofstream f;
    if ((!printNodePowerConsumption) &&
        (!printNodeEnergyConsumed) &&
        (!printComponentsPowerConsumption) &&
        (!printComponentsEnergyConsumed)
        ){
        csrr_f.Open(logName, par("outputCompression").boolValue());

    }
}

void CloudSchedulerRR::schedule (){


    if (DEBUG_CLOUD_SCHED) printf("\n Method[CLOUD_SCHEDULER_RR]: -------> schedule\n");

       //Define ...
           vector<AbstractNode*> selectedNodes;
           vector<VM*> attendedRequest_vms;
           NodeVL* node;
           Machine* machine;
           AbstractRequest* req;
           StorageRequest* req_st;
           RequestVM* req_vm;
           string uid;
           string nodeState;

           bool notEnoughResources;
           bool requestErased;
           int j;

       // Init ..

           notEnoughResources = false;
           requestErased = false;

           req = NULL;
           selectedNodes.clear();
           attendedRequest_vms.clear();
           j = 0;

       // Begin ..

           // Schedule..
           if (schedulerBlock()){

               req =  getRequestByIndex(j);
               printf("\n Method[CLOUD_SCHEDULER_RR]: ------->getrequest \n");
               // Start with the vm allocation

               while ((j < (numPendingRequests())) && (req != NULL)){
                   printf("\n Method[CLOUD_SCHEDULER_RR]: -------> inside request loop\n");
                   req_st = dynamic_cast<StorageRequest*>(req);
                   req_vm = dynamic_cast<RequestVM*>(req);

                   if (req_st != NULL){

                       machine = getNodeByIndex(req_st->getNodeSetId(),req_st->getNodeId(), false);

                       node = check_and_cast<NodeVL*>(machine);

                       // Analyzes and create the connections vector in req_st depending on the selected fs
                       // This method will invoke selectStorageNodes
                       //TODO: Return the st_req and analyzes the error at sched
                       AbstractDCManager::userStorageRequest (req_st, node);
                       eraseRequest(req_st);
                   }
                   else if (req_vm != NULL){

                       if (req->getOperation() == REQUEST_START_VM) {
                           printf("\n Method[CLOUD_SCHEDULER_RR]: -------> start vm\n");
                           notEnoughResources = request_start_vm (req_vm);
                           if (!notEnoughResources){
                               eraseRequest(req);
                               requestErased = true;
                           }
                           /*
                            * else
                            * {
                            * call timer
                            * }
                            */

                       }

                       else if (req->getOperation() == REQUEST_FREE_RESOURCES){
                           printf("\n Method[CLOUD_SCHEDULER_RR]: -------> free resources\n");
                           request_shutdown_vm(req_vm);
                           eraseRequest(req);
                           requestErased = true;
                       }

                   }
                   else if(req->getOperation() == REQUEST_ABANDON_SYSTEM){
                           // To perform management operations..
                       AbstractUser* user;
                       AbstractCloudUser* cl_user;
                       user = getUserById(req->getUid());
                       cl_user = check_and_cast<AbstractCloudUser*>(user);
                       cl_user->deleteAllVMs();
                       user->callFinish();
                       deleteUser(req->getUid());
                       eraseRequest(req);
                       requestErased = true;
                   }
                   else {
                       throw cRuntimeError("Error: Operation unknown for CloudScheduler\n");
                   }

                   if (!requestErased){
                       j++;

                   }

                   requestErased = false;
                   req = getRequestByIndex(j);

               }
               j=0;
              // vector<RunningVM*> runVM= AbstractCloudManager::runVM;
               printf("\n Method[CLOUD_SCHEDULER_RR]: -------> Before our loop\n");

               while (j<AbstractCloudManager::runVM.size())
               {
                   printf("\n Method[CLOUD_SCHEDULER_RR]: -------> enter RR\n");
                   clock_t t=clock(); // we are not sure about current time
                   RunningVM* vm;
                   vm=AbstractCloudManager::runVM.at(j);
                   if (t> vm->end_time)   // we need to shutdown vm
                   {
                       // make new request
                  //     RequestVM* reqvm;
                    //   new
                       AbstractRequest* new_req;
                   //    new new_req();



                       RequestVM* new_req_vm=new RequestVM();
                       new_req_vm->setUid(vm->userID);
                       new_req_vm->setOperation(REQUEST_START_VM);
                  //     new_req_vm->setVectorVM(vm->vmID);
                   //.    new_req_vm->setn
                    //   vm->vm->getElementType()->
                       new_req_vm->setNewSelection(vm->vm->getElementType()->getType(),1);
                     //  vm->vm->getProcessRunning()  //check  running process

/*
                       new_req->setOperation(REQUEST_START_VM);
                       new_req->setState(REQUEST_PENDING);
                       new_req->setUid(vm->user);
                    //   new_req->*/
                       new_req= dynamic_cast<AbstractRequest*>(new_req_vm);
                       // add new request to temp queue
                     //  RequestsManagement* manager=new RequestsManagement();
                      // RequestsManagement* manager;
                      // manager->u
                     //  manager->userSendRequest(new_req);
                       if (DEBUG_CLOUD_SCHED) printf("\n Method[CLOUD_SCHEDULER_RR]: -------> New Req to start VM has sent.\n");
                       // shutdown VM

                       RequestVM* new_req_vm2=new RequestVM();
                       new_req_vm2->setUid(vm->userID);
                       new_req_vm2->setOperation(REQUEST_FREE_RESOURCES);
                       new_req_vm2->setNewSelection(vm->vm->getElementType()->getType(),1); // zahra: check here first if anything wrong
                       request_shutdown_vm(new_req_vm2);
                       if (DEBUG_CLOUD_SCHED){
                           printf("\n Method[CLOUD_SCHEDULER_RR]: -------> New Req to Shutdown VM %s has been sent.\n",vm->vm->getFullName());

                       }

                       // save the state


                       // erase from vector
                       AbstractCloudManager::runVM.erase(AbstractCloudManager::runVM.begin()+j);

                   }
                   else
                   {
                       ++j;
                   }
               }

               schedulerUnblock();
           }

}


AbstractNode* CloudSchedulerRR::selectNode (AbstractRequest* req){
	if (DEBUG_CLOUD_SCHED) printf("\n Method[SCHEDULER_ROUNDROBIN]: -------> select_nodes\n");

	// Define ..
	        AbstractNode* node;
	        int vmCPU;
	        int vmMemory;
	        elementType* el;
	        RequestVM* reqVm;
	        int positionInitial;
	        int setInitial;
	        bool found;
	        int numProcesses;

        // Cast
        reqVm = dynamic_cast<RequestVM*>(req);
        if (reqVm == NULL) throw cRuntimeError("AbstractCloudManager::selectNode->Error. Casting the request\n");

    // Init ..
        node = NULL;
        el = reqVm->getSingleRequestType();
        vmCPU = el->getNumCores();
        vmMemory = el->getMemorySize();
        setInitial = currentNodeType;
        positionInitial = currentNodeIndex;
        found = false;

    // Begin ..

        // select the node

            positionInitial = currentNodeIndex;
            while (!found){

                 node = getNodeByIndex(setInitial,positionInitial);

                 if ( (node->getFreeMemory() >= vmMemory) && (node->getNumCores() >= vmCPU)){

                     NodeVL* node_vl = check_and_cast<NodeVL*>(node);
                     numProcesses = node_vl->getNumOfLinkedVMs();
                     if (numProcesses < maximum_number_of_processes_per_node){
                         node = check_and_cast<Node*>(node_vl);
                     }

                     found = true;
                 }

                 currentNodeIndex++;

                 if ((unsigned int)currentNodeIndex > (unsigned int)getSetSize(setInitial)){
                     setInitial++;
                     if (setInitial < getMapSize()){
                         currentNodeType = setInitial;
                     }
                     else{
                        currentNodeType = 0;
                     }
                     currentNodeIndex = 0;
                  }

                 // The algorithm has travel by all the values and it not reach a solution. So, the node is null.
                 if ((positionInitial == currentNodeIndex) && (currentNodeType == setInitial)){
                     found = true;
                     node = NULL;
                 }
            }

		return node;

}

vector<AbstractNode*> CloudSchedulerRR::selectStorageNodes (AbstractRequest* st_req){

    if (DEBUG_CLOUD_SCHED) printf("\n Method[SCHEDULER_RR]: -------> selectStorageNode\n");

    // Define ..
        int numNodesFs;
        int i,j;
        vector<int> selected_nodes;
        vector<AbstractNode*> nodes;
        AbstractNode* node;
        int diskCapacity;
        int diskFree;
        StorageRequest* streq;

    // Initialize ..
        selected_nodes.clear();
        nodes.clear();
        streq = dynamic_cast<StorageRequest*>(st_req);
        if (streq == NULL) throw cRuntimeError("CloudSchedulerRR::selectStorageNodes ->can not cast to storage request\n");

    // Select the number of nodes depending of the type of filesystem
        if (strcmp (streq->getFsType().c_str(), FS_NFS) == 0){
            numNodesFs = 1;
        }
        else if (strcmp (streq->getFsType().c_str(), FS_PFS) == 0){
            numNodesFs = numberOfPFSRemoteServers;
        }

        if (numNodesFs > getStorageNodesSize())
            throw cRuntimeError("[CloudSchedulerRR::selectStorageNodes] -> Trying to instantiate a file system that requires more disks than nodes.. (actually 1 disk device per node) ..\n");

        // Select the nodes

        i = 0;

        for (j = 0; (j < AllocationManagement::getStorageNodesSize()) && (i < numNodesFs); j++){

            node = AllocationManagement::getStorageNode(j);
            diskFree = node->getFreeStorage();
            diskCapacity = node->getStorageCapacity();

            if (diskFree >= (diskCapacity*minimum_percent_storageNode_free)){
                i++;
                selected_nodes.push_back(j);
                nodes.push_back(node);
            }

        }

        if (i != numNodesFs){
            nodes.clear();
        }


    // It is needed to update the structures ..
    if (selected_nodes.size() != 0)
        updateVmManagementStructures(selected_nodes, st_req->getPid(), st_req->getUid());

    return nodes;

}

vector<AbstractNode*> CloudSchedulerRR::remoteShutdown (AbstractRequest* req){

    // Define ..
        vector<AbstractNode*> nodes;
        RequestVM* reqVm;

    // Init ..
        nodes.clear();

     // Cast
        reqVm = dynamic_cast<RequestVM*>(req);
        if (reqVm == NULL) throw cRuntimeError("AbstractCloudManager::remoteShutdown->Error. Casting the request\n");


    // Search the user entry if exists
        int position;
        position = searchUserVMAllocation(reqVm->getUid());


        // If it exists an entry, it means that the user has remote storage
        if (position != -1)
            nodes = deleteVMfromStorageNodes(position, req->getPid());

        else
            nodes.clear();


        return nodes;

}

void CloudSchedulerRR::freeResources (int uId, int pId, AbstractNode* computingNode){
//void CloudSchedulerRR::freeResources(VM* vm, AbstractNode* computingNode){

}

void CloudSchedulerRR::printEnergyValues(){

    // Define ..
    int i, j;
    AbstractNode* nodeA;
    Node* node;
    ostringstream data;
    ostringstream file;
    int computeNodeMapSize;
    int computeNodeSetSize;
    int storageNodeMapSize;
    int storageNodeSetSize;

    double nodeEnergyConsumed = 0.0;
    double nodeInstantConsumption = 0.0;
    double cpuEnergyConsumed = 0.0;
    double cpuInstantConsumption = 0.0;
    double memoryEnergyConsumed = 0.0;
    double memoryInstantConsumption = 0.0;
    double nicEnergyConsumed = 0.0;
    double nicInstantConsumption = 0.0;
    double storageEnergyConsumed = 0.0;
    double storageInstantConsumption = 0.0;
    double psuEnergyConsumed = 0.0;
    double psuInstantConsumption = 0.0;
    double dataCenterInstantConsumption = 0.0;

    // Init..
        computeNodeMapSize = getMapSize();
        storageNodeMapSize = getStorageMapSize();

    if ((printNodePowerConsumption) || (printNodeEnergyConsumed) ||
        (printComponentsEnergyConsumed) || (printComponentsPowerConsumption) ||
        (printDataCenterPowerConsumption)  || (printDataCenterEnergyConsumed)
        )  data << simTime();

    // Compute nodes
    for (i = 0; i < computeNodeMapSize; i++){

        computeNodeSetSize = getSetSize(i, false);

       for (j = 0; j < computeNodeSetSize; j++){

           nodeA = getNodeByIndex(i,j, false);
           node = dynamic_cast<Node*>(nodeA);

           // Get all the data to variables

           if (printComponentsPowerConsumption){
               cpuInstantConsumption = node->getCPUInstantConsumption();
               memoryInstantConsumption = node->getMemoryInstantConsumption();
               storageInstantConsumption = node->getStorageInstantConsumption();
               nicInstantConsumption = node->getNICInstantConsumption();
               psuInstantConsumption = node->getPSUConsumptionLoss();
           }

           if (printComponentsEnergyConsumed){
               cpuEnergyConsumed = node->getCPUEnergyConsumed();
               memoryEnergyConsumed = node->getMemoryEnergyConsumed();
               storageEnergyConsumed = node->getStorageEnergyConsumed();
               nicEnergyConsumed = node->getNICEnergyConsumed();
               psuEnergyConsumed = node->getPSUConsumptionLoss();
           }

           if (printNodePowerConsumption){
               if (!printComponentsPowerConsumption)
                   nodeInstantConsumption = node->getInstantConsumption();
               else
                   nodeInstantConsumption =  cpuInstantConsumption + memoryInstantConsumption + storageInstantConsumption+ nicInstantConsumption + psuInstantConsumption;
           }

           if (printNodeEnergyConsumed){
               if (!printComponentsEnergyConsumed)
                   nodeEnergyConsumed = node->getEnergyConsumed();
               else
                   nodeEnergyConsumed =  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
           }

           if(printDataCenterPowerConsumption){
               if (printNodePowerConsumption){
                   dataCenterInstantConsumption += nodeInstantConsumption;
               }
               else{
                   if (printComponentsPowerConsumption){
                       dataCenterInstantConsumption +=  cpuInstantConsumption + memoryInstantConsumption + storageInstantConsumption+ nicInstantConsumption + psuInstantConsumption;
                   } else{
                       dataCenterInstantConsumption += node->getInstantConsumption();
                   }
               }
           }

           if(printDataCenterEnergyConsumed){
               if (printNodeEnergyConsumed){
                   dc_EnergyConsumed += nodeEnergyConsumed;
               }
               else{
                   if (printComponentsEnergyConsumed){
                       dc_EnergyConsumed +=  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
                   } else{
                       dc_EnergyConsumed += node->getEnergyConsumed();
                   }
               }
           }

           if (printEnergyTrace) {
               if ((printNodePowerConsumption) || (printNodeEnergyConsumed) || (printComponentsPowerConsumption) || (printComponentsPowerConsumption))
                   data  << endl << node->getFullName() << ";" << node->getState();
               if (printNodePowerConsumption)
                   data << ";(nW)" << nodeInstantConsumption;
               if (printNodeEnergyConsumed)
                   data << ";(nJ)" << nodeEnergyConsumed;
               if (printComponentsPowerConsumption)
                   data << ";(cW)" << cpuInstantConsumption << ";" << memoryInstantConsumption << ";" << nicInstantConsumption << ";" <<  storageInstantConsumption << ";" << psuInstantConsumption;
               if (printComponentsPowerConsumption)
                   data << ";(cJ)" << cpuEnergyConsumed << ";" << memoryEnergyConsumed << ";" << nicEnergyConsumed << ";" <<  storageEnergyConsumed << ";" << psuEnergyConsumed;
           }
       }
   }

    // Storage nodes
    for (i = 0; i < storageNodeMapSize; i++){

       storageNodeSetSize = getSetSize(i, true);

       for (j = 0; j < storageNodeSetSize; j++){

           nodeA = getNodeByIndex(i,j, true);
           node = dynamic_cast<Node*>(nodeA);

           // Get all the data to variables

             if (printComponentsPowerConsumption){
                 cpuInstantConsumption = node->getCPUInstantConsumption();
                 memoryInstantConsumption = node->getMemoryInstantConsumption();
                 storageInstantConsumption = node->getStorageInstantConsumption();
                 nicInstantConsumption = node->getNICInstantConsumption();
                 psuInstantConsumption = node->getPSUConsumptionLoss();

             }

             if (printComponentsEnergyConsumed){
                 cpuEnergyConsumed = node->getCPUEnergyConsumed();
                 memoryEnergyConsumed = node->getMemoryEnergyConsumed();
                 storageEnergyConsumed = node->getStorageEnergyConsumed();
                 nicEnergyConsumed = node->getNICEnergyConsumed();
                 psuEnergyConsumed = node->getPSUConsumptionLoss();
             }

             if (printNodePowerConsumption){
                 if (!printComponentsPowerConsumption)
                     nodeInstantConsumption = node->getInstantConsumption();
                 else
                     nodeInstantConsumption =  cpuInstantConsumption + memoryInstantConsumption + storageInstantConsumption+ nicInstantConsumption + psuInstantConsumption;
             }

             if (printNodeEnergyConsumed){
                 if (!printComponentsEnergyConsumed)
                     nodeEnergyConsumed = node->getEnergyConsumed();
                 else
                     nodeEnergyConsumed =  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
             }

             if(printDataCenterPowerConsumption){
                 if (printNodePowerConsumption){
                     dataCenterInstantConsumption += nodeInstantConsumption;
                 }
                 else{
                     if (printComponentsPowerConsumption){
                         dataCenterInstantConsumption +=  cpuInstantConsumption + memoryInstantConsumption + storageInstantConsumption+ nicInstantConsumption + psuInstantConsumption;
                     } else{
                         dataCenterInstantConsumption += node->getInstantConsumption();
                     }
                 }
             }

             if(printDataCenterEnergyConsumed){
                 if (printNodeEnergyConsumed){
                     dc_EnergyConsumed += nodeEnergyConsumed;
                 }
                 else{
                     if (printComponentsEnergyConsumed){
                         dc_EnergyConsumed +=  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
                     } else{
                         dc_EnergyConsumed += node->getEnergyConsumed();
                     }
                 }
             }

             if (printEnergyTrace) {
                if ((printNodePowerConsumption) || (printNodeEnergyConsumed) || (printComponentsPowerConsumption) || (printComponentsPowerConsumption))
                    data  << endl << node->getFullName() << ";" << node->getState();
                if (printNodePowerConsumption)
                    data << ";(nW)" << nodeInstantConsumption;
                if (printNodeEnergyConsumed)
                    data << ";(nJ)" << nodeEnergyConsumed;
                if (printComponentsPowerConsumption)
                    data << ";(cW)" << cpuInstantConsumption << ";" << memoryInstantConsumption << ";" << nicInstantConsumption << ";" <<  storageInstantConsumption << ";" << psuInstantConsumption;
                if (printComponentsPowerConsumption)
                    data << ";(cJ)" << cpuEnergyConsumed << ";" << memoryEnergyConsumed << ";" << nicEnergyConsumed << ";" <<  storageEnergyConsumed << ";" << psuEnergyConsumed;
             }
       }
   }

    if ((printDataCenterPowerConsumption)  || (printDataCenterEnergyConsumed)){
        data << endl << "#";
        if (printDataCenterPowerConsumption){
            data << dataCenterInstantConsumption;
        }

        if (printDataCenterEnergyConsumed){
            if (printDataCenterPowerConsumption) data << ";";
            data << dc_EnergyConsumed << endl;
        }
    }

    // print data to the file
        csrr_f.Append(data.str().c_str()) ;

}

void CloudSchedulerRR::finalizeScheduler(){
    // Define ..
      AbstractNode* nodeA;
      Node* node;
      ostringstream data;
      ostringstream file;
      int i,j;
      int computeNodeMapSize;
      int storageNodeMapSize;
      int storageNodeSetSize;

      int computeNodeSetSize = 0;
      int storageSetSize = 0;
      int totalNumberNodes = 0;

      double nodeEnergyConsumed = 0.0;
      double cpuEnergyConsumed = 0.0;
      double memoryEnergyConsumed = 0.0;
      double nicEnergyConsumed = 0.0;
      double storageEnergyConsumed = 0.0;
      double psuEnergyConsumed = 0.0;
      double dataCenterEnergyConsumed = 0.0;
      vector<HeterogeneousSet*>::iterator setIt;

    if (printEnergyToFile){

        // Print the totals
            if (!printEnergyTrace){

              // Compute nodes
                  for (i = 0; i < getMapSize(); i++)
                      computeNodeSetSize += getSetSize(i, false);

              // Storage nodes
                  for (i = 0; i < getStorageMapSize(); i++)
                      storageSetSize += getSetSize(i, true);

                  totalNumberNodes = computeNodeSetSize + storageSetSize;

              // print the mode and the number of nodes
              file << "@Total-mode;" << totalNumberNodes << endl;
              file << simTime() << endl;

              // Init..
                  computeNodeMapSize = getMapSize();
                  storageNodeMapSize = getStorageMapSize();

              // Compute nodes
              for (i = 0; i < computeNodeMapSize; i++){

                  computeNodeSetSize = getSetSize(i, false);

                 for (j = 0; j < computeNodeSetSize; j++){

                     nodeA = getNodeByIndex(i,j, false);
                     node = dynamic_cast<Node*>(nodeA);
                 // Get all the data to variables
                     cpuEnergyConsumed = node->getCPUEnergyConsumed();
                     memoryEnergyConsumed = node->getMemoryEnergyConsumed();
                     storageEnergyConsumed = node->getStorageEnergyConsumed();
                     nicEnergyConsumed = node->getNICEnergyConsumed();
                     psuEnergyConsumed = node->getPSUConsumptionLoss();

                     nodeEnergyConsumed =  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
                     data  << node->getFullName() << ";" << nodeEnergyConsumed << endl;
                     dataCenterEnergyConsumed += nodeEnergyConsumed;
                  }
             }

              // Storage nodes
              for (i = 0; i < storageNodeMapSize; i++){

                 storageNodeSetSize = getSetSize(i, true);

                 for (j = 0; j < storageNodeSetSize; j++){
                     nodeA = getNodeByIndex(i,j, true);
                     node = dynamic_cast<Node*>(nodeA);

                 // Get all the data to variables
                     cpuEnergyConsumed = node->getCPUEnergyConsumed();
                     memoryEnergyConsumed = node->getMemoryEnergyConsumed();
                     storageEnergyConsumed = node->getStorageEnergyConsumed();
                     nicEnergyConsumed = node->getNICEnergyConsumed();
                     psuEnergyConsumed = node->getPSUConsumptionLoss();

                     nodeEnergyConsumed =  cpuEnergyConsumed + memoryEnergyConsumed + storageEnergyConsumed+ nicEnergyConsumed + psuEnergyConsumed;
                     data << node->getFullName() << ";" << nodeEnergyConsumed << endl;
                     dataCenterEnergyConsumed += nodeEnergyConsumed;
                 }
              }

              data << "#" << dataCenterEnergyConsumed << endl;

              // print data to the file
                csrr_f.Append(file.str().c_str()) ;
                csrr_f.Close();
            }
    }

}

int CloudSchedulerRR::selectNodeSet (string setName, int vmcpu, int vmmemory){

    if (DEBUG_CLOUD_SCHED) printf("\n Method[SCHEDULER_FIFO]: -------> selectNodeSet\n");

    int bestFit;
    int acumCPU;
//  int acumVM;
    int i;
    int numTypeSize;
    int numCPUs;

    // Init ..
    bestFit = -1;
    acumCPU = 1024;
    numCPUs = -1;
    numTypeSize = getMapSize();
    //acumVM = set.begin()->getTotalMemory();

    for (i = 0; i < numTypeSize; i++){
        numCPUs = getSetNumCores(i, false);
        if (((numCPUs - vmcpu) < acumCPU )){ //&& ((set.begin()->getTotalMemory - acumVM )  < vmmemory)){

            bestFit  = i;
            acumCPU  = numCPUs - vmcpu;
            //vmmemory = bestFit -> getTotalMemory - acumVM;
        }
    }

    return bestFit;

}

