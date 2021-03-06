using System;
using System.Collections.Generic;
using System.Text;
/* poll_subscriber.cs

   A subscription example

   This file is derived from code automatically generated by the rtiddsgen 
   command:

   rtiddsgen -language C# -example <arch> poll.idl

   Example subscription of type poll automatically generated by 
   'rtiddsgen'. To test them, follow these steps:

   (1) Compile this file and the example publication.

   (2) Start the subscription with the command
       objs\<arch>\poll_subscriber <domain_id> <sample_count>

   (3) Start the publication with the command
       objs\<arch>\poll_publisher <domain_id> <sample_count>

   (4) [Optional] Specify the list of discovery initial peers and 
       multicast receive addresses via an environment variable or a file 
       (in the current working directory) called NDDS_DISCOVERY_PEERS. 

   You can run any number of publishers and subscribers programs, and can 
   add and remove them dynamically from the domain.
                                   
   Example:
        
       To run the example application on domain <domain_id>:
                          
       bin\<Debug|Release>\poll_publisher <domain_id> <sample_count>  
       bin\<Debug|Release>\poll_subscriber <domain_id> <sample_count>
              
       
modification history
------------ -------
*/

public class pollSubscriber {



    public static void Main(string[] args) {

        // --- Get domain ID --- //
        int domain_id = 0;
        if (args.Length >= 1) {
            domain_id = Int32.Parse(args[0]);
        }

        // --- Get max loop count; 0 means infinite loop  --- //
        int sample_count = 0;
        if (args.Length >= 2) {
            sample_count = Int32.Parse(args[1]);
        }

        /* Uncomment this to turn on additional logging
        NDDS.ConfigLogger.get_instance().set_verbosity_by_category(
            NDDS.LogCategory.NDDS_CONFIG_LOG_CATEGORY_API, 
            NDDS.LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
        */

        // --- Run --- //
        try {
            pollSubscriber.subscribe(
                domain_id, sample_count);
        }
        catch(DDS.Exception) {
            Console.WriteLine("error in subscriber");
        }
    }

    static void subscribe(int domain_id, int sample_count) {

        // --- Create participant --- //

        /* To customize the participant QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.DomainParticipant participant =
            DDS.DomainParticipantFactory.get_instance().create_participant(
                domain_id,
                DDS.DomainParticipantFactory.PARTICIPANT_QOS_DEFAULT, 
                null /* listener */,
                DDS.StatusMask.STATUS_MASK_NONE);
        if (participant == null) {
            shutdown(participant);
            throw new ApplicationException("create_participant error");
        }

        // --- Create subscriber --- //

        /* To customize the subscriber QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.Subscriber subscriber = participant.create_subscriber(
            DDS.DomainParticipant.SUBSCRIBER_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (subscriber == null) {
            shutdown(participant);
            throw new ApplicationException("create_subscriber error");
        }

        // --- Create topic --- //

        /* Register the type before creating the topic */
        System.String type_name = pollTypeSupport.get_type_name();
        try {
            pollTypeSupport.register_type(
                participant, type_name);
        }
        catch(DDS.Exception e) {
            Console.WriteLine("register_type error {0}", e);
            shutdown(participant);
            throw e;
        }

        /* To customize the topic QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.Topic topic = participant.create_topic(
            "Example poll",
            type_name,
            DDS.DomainParticipant.TOPIC_QOS_DEFAULT,
            null /* listener */,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (topic == null) {
            shutdown(participant);
            throw new ApplicationException("create_topic error");
        }

        // --- Create reader --- //

        /* To customize the data reader QoS, use 
           the configuration file USER_QOS_PROFILES.xml */
        DDS.DataReader reader = subscriber.create_datareader(
            topic,
            DDS.Subscriber.DATAREADER_QOS_DEFAULT,
            null,
            DDS.StatusMask.STATUS_MASK_NONE);
        if (reader == null) {
            shutdown(participant);
            throw new ApplicationException("create_datareader error");
        }

        /* If you want to change datareader_qos.history.kind programmatically rather
         * than using the XML file, you will need to add the following lines to your
         * code and comment out the create_datareader call above. */

        /*DDS.DataReaderQos datareader_qos = new DDS.DataReaderQos();

        try
        {
            subscriber.get_default_datareader_qos(datareader_qos);
        }
        catch (DDS.Exception e)
        {
            Console.WriteLine("get_default_datareader_qos error {0}", e);
            shutdown(participant);
            throw e;
        }

        datareader_qos.history.kind = 
            DDS.HistoryQosPolicyKind.KEEP_ALL_HISTORY_QOS;

        DDS.DataReader reader = subscriber.create_datareader(
            topic, datareader_qos, null,
            DDS.StatusMask.STATUS_MASK_ALL);
        if (reader == null) {
            shutdown(participant);
            throw new ApplicationException("create_datareader error");
        }
        */


        pollDataReader poll_reader =
            (pollDataReader)reader;


        // --- Wait for data --- //
        DDS.SampleInfoSeq info_seq = new DDS.SampleInfoSeq();
        pollSeq data_seq = new pollSeq();

        /* Main loop */
        const System.Int32 receive_period = 5000; // milliseconds
        for (int count=0;
             (sample_count == 0) || (count < sample_count);
             ++count) {
            Console.WriteLine(
                "poll subscriber sleeping for {0} sec...",
                receive_period / 1000);

            System.Threading.Thread.Sleep(receive_period);

            // --- Polling for data --- ///

            /* Check for new data calling the DataReader's take() method */
            try
            {
                poll_reader.take(
                data_seq, info_seq, DDS.ResourceLimitsQosPolicy.LENGTH_UNLIMITED,
                DDS.SampleStateKind.ANY_SAMPLE_STATE,
                DDS.ViewStateKind.ANY_VIEW_STATE, DDS.
                InstanceStateKind.ANY_INSTANCE_STATE);
            }
            catch (DDS.Retcode_NoData)
            {
                // Not an error 
                return;
            }
            catch (DDS.Exception e)
            {
                Console.WriteLine("take error {0}", e);
                return;
            }

            int len = 0;
            double sum = 0;

            /* Iterate through the samples read using the take() method, getting
             * the number of samples got and, adding the value of x on each of
             * them to calculate the average afterwards. */
            for (int i = 0; i < data_seq.length; ++i)
            {
                if (!info_seq.get_at(i).valid_data)
                    continue;
                len++;
                sum += data_seq.get_at(i).x;
            }

            if (len > 0)
            {
                Console.WriteLine("Got {0} samples.  Avg = {1}", len, sum / len);
            }

            try
            {
                poll_reader.return_loan(data_seq, info_seq);
            }
            catch (DDS.Exception e)
            {
                Console.WriteLine("return loan error {0}", e);
            }

        }

        // --- Shutdown --- //

        /* Delete all entities */
        shutdown(participant);
    }


    static void shutdown(
        DDS.DomainParticipant participant) {

        /* Delete all entities */

        if (participant != null) {
            participant.delete_contained_entities();
            DDS.DomainParticipantFactory.get_instance().delete_participant(
                ref participant);
        }

        /* RTI Connext provides finalize_instance() method on
           domain participant factory for users who want to release memory
           used by the participant factory. Uncomment the following block of
           code for clean destruction of the singleton. */
        /*
        try {
            DDS.DomainParticipantFactory.finalize_instance();
        }
        catch(DDS.Exception e) {
            Console.WriteLine("finalize_instance error {0}", e);
            throw e;
        }
        */
    }
}


