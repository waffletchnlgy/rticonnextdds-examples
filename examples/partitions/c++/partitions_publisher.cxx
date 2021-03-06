/* partitions_publisher.cxx

   A publication of data of type partitions

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C++ -example <arch> partitions.idl

   Example publication of type partitions automatically generated by 
   'rtiddsgen'. To test them follow these steps:

   (1) Compile this file and the example subscription.

   (2) Start the subscription with the command
       objs/<arch>/partitions_subscriber <domain_id> <sample_count>
                
   (3) Start the publication with the command
       objs/<arch>/partitions_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 
       
   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.

                                   
   Example:
        
       To run the example application on domain <domain_id>:
                          
       On Unix: 
       
       objs/<arch>/partitions_publisher <domain_id> o
       objs/<arch>/partitions_subscriber <domain_id> 
                            
       On Windows:
       
       objs\<arch>\partitions_publisher <domain_id>  
       objs\<arch>\partitions_subscriber <domain_id>    

       
modification history
------------ -------       
*/

#include <stdio.h>
#include <stdlib.h>
#ifdef RTI_VX653
#include <vThreadsData.h>
#endif
#include "partitions.h"
#include "partitionsSupport.h"
#include "ndds/ndds_cpp.h"

/* Delete all entities */
static int publisher_shutdown(
    DDSDomainParticipant *participant)
{
    DDS_ReturnCode_t retcode;
    int status = 0;

    if (participant != NULL) {
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_contained_entities error %d\n", retcode);
            status = -1;
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            printf("delete_participant error %d\n", retcode);
            status = -1;
        }
    }

    /* RTI Connext provides finalize_instance() method on
       domain participant factory for people who want to release memory used
       by the participant factory. Uncomment the following block of code for
       clean destruction of the singleton. */
/*
    retcode = DDSDomainParticipantFactory::finalize_instance();
    if (retcode != DDS_RETCODE_OK) {
        printf("finalize_instance error %d\n", retcode);
        status = -1;
    }
*/

    return status;
}

extern "C" int publisher_main(int domainId, int sample_count)
{
    DDSDomainParticipant *participant = NULL;
    DDSPublisher *publisher = NULL;
    DDSTopic *topic = NULL;
    DDSDataWriter *writer = NULL;
    partitionsDataWriter * partitions_writer = NULL;
    partitions *instance = NULL;
    DDS_ReturnCode_t retcode;
    DDS_InstanceHandle_t instance_handle = DDS_HANDLE_NIL;
    const char *type_name = NULL;
    int count = 0;  
    DDS_Duration_t send_period = {1,0};

    /* To customize participant QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    participant = DDSTheParticipantFactory->create_participant(
        domainId, DDS_PARTICIPANT_QOS_DEFAULT, 
        NULL /* listener */, DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        printf("create_participant error\n");
        publisher_shutdown(participant);
        return -1;
    }
    
    DDS_PublisherQos publisher_qos;
    retcode = participant->get_default_publisher_qos(publisher_qos);
    if (retcode != DDS_RETCODE_OK) {
        printf("get_default_publisher_qos error\n");
        return -1;
    }

    /* If you want to change the Partition name programmatically rather than
     * using the XML, you will need to add the following lines to your code
     * and comment out the create_publisher() call bellow.
     */
    publisher_qos.partition.name.ensure_length(2, 2);
    publisher_qos.partition.name[0] = DDS_String_dup("ABC");
    publisher_qos.partition.name[1] = DDS_String_dup("foo");
    
    printf("Setting partition to '%s', '%s'...\n",
           publisher_qos.partition.name[0],
           publisher_qos.partition.name[1]);

    publisher = participant->create_publisher(publisher_qos, 
					      NULL, 
					      DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
      printf("create_participant error\n");
      publisher_shutdown(participant);
      return -1;
    }
    
    /*
    publisher = participant->create_publisher(DDS_PUBLISHER_QOS_DEFAULT, 
					      NULL, 
					      DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        printf("create_publisher error\n");
        publisher_shutdown(participant);
        return -1;
    }
    */

    /* Register type before creating topic */
    type_name = partitionsTypeSupport::get_type_name();
    retcode = partitionsTypeSupport::register_type(
        participant, type_name);
    if (retcode != DDS_RETCODE_OK) {
        printf("register_type error %d\n", retcode);
        publisher_shutdown(participant);
        return -1;
    }

    /* To customize topic QoS, use 
       the configuration file USER_QOS_PROFILES.xml */
    topic = participant->create_topic(
        "Example partitions",
        type_name, DDS_TOPIC_QOS_DEFAULT, NULL /* listener */,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        printf("create_topic error\n");
        publisher_shutdown(participant);
        return -1;
    }


    /* In this example we set a Reliable datawriter, with Transient Local 
     * durability. By default we set up these QoS settings via XML. If you
     * want to to it programmatically, use the following code, and comment out
     * the create_datawriter call bellow.
     */
    /*
    DDS_DataWriterQos datawriter_qos;
    retcode = publisher->get_default_datawriter_qos(datawriter_qos);
    if (retcode != DDS_RETCODE_OK) {
      printf("get_default_datawriter_qos error\n");
      return -1;
    }
    datawriter_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    datawriter_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
    datawriter_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
    datawriter_qos.history.depth = 3;
    
    writer = publisher->create_datawriter(topic, 
					  datawriter_qos, 
					  NULL,
					  DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
      printf("create_datawriter error\n");
      publisher_shutdown(participant);
      return -1;
    }
    */
    
    writer = publisher->create_datawriter(topic, DDS_DATAWRITER_QOS_DEFAULT, NULL,
        DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
        printf("create_datawriter error\n");
        publisher_shutdown(participant);
        return -1;
    }
    partitions_writer = partitionsDataWriter::narrow(writer);
    if (partitions_writer == NULL) {
        printf("DataWriter narrow error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* Create data sample for writing */

    instance = partitionsTypeSupport::create_data();
    
    if (instance == NULL) {
        printf("partitionsTypeSupport::create_data error\n");
        publisher_shutdown(participant);
        return -1;
    }

    /* For a data type that has a key, if the same instance is going to be
       written multiple times, initialize the key here
       and register the keyed instance prior to writing */
/*
    instance_handle = partitions_writer->register_instance(*instance);
*/

    /* Main loop */
    for (count=0; (sample_count == 0) || (count < sample_count); ++count) {
      printf("Writing partitions, count %d\n", count);
      instance->x = count;
      
      retcode = partitions_writer->write(*instance, instance_handle);
      if (retcode != DDS_RETCODE_OK) {
	printf("write error %d\n", retcode);
      }

      /* Every 5 samples we will change the Partition name. These are the
       * partition expressions we are going to try: 
       * "bar", "A*", "A?C", "X*Z", "zzz", "A*C"
       */
      if ((count+1) % 25 == 0) {
	// Matches "ABC" -- name[1] here can match name[0] there, 
	// as long as there is some overlapping name
	publisher_qos.partition.name[0] = DDS_String_dup("zzz");
	publisher_qos.partition.name[1] = DDS_String_dup("A*C");
	printf("Setting partition to '%s', '%s'...\n",
	       publisher_qos.partition.name[0],
	       publisher_qos.partition.name[1]);
	publisher->set_qos(publisher_qos);
      }
      else if ((count+1) % 20 == 0) { 
	// Strings that are regular expressions aren't tested for
	// literal matches, so this won't match "X*Z"
	publisher_qos.partition.name[0] = DDS_String_dup("X*Z");
	printf("Setting partition to '%s', '%s'...\n",
	       publisher_qos.partition.name[0],
	       publisher_qos.partition.name[1]);
	publisher->set_qos(publisher_qos);	
      }
      else if ((count+1) % 15 == 0) {
	// Matches "ABC"
	publisher_qos.partition.name[0] = DDS_String_dup("A?C");
	printf("Setting partition to '%s', '%s'...\n",
	       publisher_qos.partition.name[0],
	       publisher_qos.partition.name[1]);
	publisher->set_qos(publisher_qos);
      }
      else if ((count+1) % 10 == 0) {
	// Matches "ABC"
	publisher_qos.partition.name[0] = DDS_String_dup("A*");
	printf("Setting partition to '%s', '%s'...\n",
	       publisher_qos.partition.name[0],
	       publisher_qos.partition.name[1]);
	publisher->set_qos(publisher_qos);
      }
      else if ((count+1) % 5 == 0) {
	// No literal match for "bar"
	publisher_qos.partition.name[0] = DDS_String_dup("bar");
	printf("Setting partition to '%s', '%s'...\n",
	       publisher_qos.partition.name[0],
	       publisher_qos.partition.name[1]);
	publisher->set_qos(publisher_qos);
      }
            
      NDDSUtility::sleep(send_period);
    }

/*
    retcode = partitions_writer->unregister_instance(
        *instance, instance_handle);
    if (retcode != DDS_RETCODE_OK) {
        printf("unregister instance error %d\n", retcode);
    }
*/

    /* Delete data sample */
    retcode = partitionsTypeSupport::delete_data(instance);
    if (retcode != DDS_RETCODE_OK) {
        printf("partitionsTypeSupport::delete_data error %d\n", retcode);
    }

    /* Delete all entities */
    return publisher_shutdown(participant);
}

#if defined(RTI_WINCE)
int wmain(int argc, wchar_t** argv)
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */ 
    
    if (argc >= 2) {
        domainId = _wtoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = _wtoi(argv[2]);
    }

     /* Uncomment this to turn on additional logging
    NDDSConfigLogger::get_instance()->
        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */
    
    return publisher_main(domainId, sample_count);
}
 
#elif !(defined(RTI_VXWORKS) && !defined(__RTP__)) && !defined(RTI_PSOS)
int main(int argc, char *argv[])
{
    int domainId = 0;
    int sample_count = 0; /* infinite loop */

    if (argc >= 2) {
        domainId = atoi(argv[1]);
    }
    if (argc >= 3) {
        sample_count = atoi(argv[2]);
    }

    /* Uncomment this to turn on additional logging
    NDDSConfigLogger::get_instance()->
        set_verbosity_by_category(NDDS_CONFIG_LOG_CATEGORY_API, 
                                  NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
    */
    
    return publisher_main(domainId, sample_count);
}
#endif

#ifdef RTI_VX653
const unsigned char* __ctype = *(__ctypePtrGet());

extern "C" void usrAppInit ()
{
#ifdef  USER_APPL_INIT
    USER_APPL_INIT;         /* for backwards compatibility */
#endif
    
    /* add application specific code here */
    taskSpawn("pub", RTI_OSAPI_THREAD_PRIORITY_NORMAL, 0x8, 0x150000, (FUNCPTR)publisher_main, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
   
}
#endif

