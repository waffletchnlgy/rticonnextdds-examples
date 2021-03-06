/* cfcSubscriber.java

 A publication of data of type cfc

 This file is derived from code automatically generated by the rtiddsgen 
 command:

 rtiddsgen -language java -example <arch> .idl

 Example publication of type cfc automatically generated by 
 'rtiddsgen' To test them follow these steps:

 (1) Compile this file and the example subscription.

 (2) Start the subscription on the same domain used for with the command
 java cfcSubscriber <domain_id> <sample_count>

 (3) Start the publication with the command
 java cfcPublisher <domain_id> <sample_count>

 (4) [Optional] Specify the list of discovery initial peers and 
 multicast receive addresses via an environment variable or a file 
 (in the current working directory) called NDDS_DISCOVERY_PEERS. 

 You can run any number of publishers and subscribers programs, and can 
 add and remove them dynamically from the domain.


 Example:

 To run the example application on domain <domain_id>:

 Ensure that $(NDDSHOME)/lib/<arch> is on the dynamic library path for
 Java.                       

 On UNIX systems: 
 add $(NDDSHOME)/lib/<arch> to the 'LD_LIBRARY_PATH' environment
 variable

 On Windows systems:
 add %NDDSHOME%\lib\<arch> to the 'Path' environment variable


 Run the Java applications:

 java -Djava.ext.dirs=$NDDSHOME/class cfcPublisher <domain_id>

 java -Djava.ext.dirs=$NDDSHOME/class cfcSubscriber <domain_id>  


 modification history
 ------------ -------   
 */

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.DecimalFormat;
import java.util.Arrays;

import com.rti.dds.domain.*;
import com.rti.dds.infrastructure.*;
import com.rti.dds.subscription.*;
import com.rti.dds.topic.*;
import com.rti.ndds.config.*;

// ===========================================================================

public class cfcSubscriber {
    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    public static void main(String[] args) {
        // --- Get domain ID --- //
        int domainId = 0;
        if (args.length >= 1) {
            domainId = Integer.valueOf(args[0]).intValue();
        }

        // -- Get max loop count; 0 means infinite loop --- //
        int sampleCount = 0;
        if (args.length >= 2) {
            sampleCount = Integer.valueOf(args[1]).intValue();
        }

        /*
         * Uncomment this to turn on additional logging
         * Logger.get_instance().set_verbosity_by_category(
         * LogCategory.NDDS_CONFIG_LOG_CATEGORY_API,
         * LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
         */

        // --- Run --- //
        subscriberMain(domainId, sampleCount);
    }

    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------

    // --- Constructors: -----------------------------------------------------

    private cfcSubscriber() {
        super();
    }

    // -----------------------------------------------------------------------

    private static void subscriberMain(int domainId, int sampleCount) {

        DomainParticipant participant = null;
        Subscriber subscriber = null;
        Topic topic = null;
        DataReaderListener listener = null;
        cfcDataReader reader = null;

        try {

            // --- Create participant --- //

            /*
             * To customize participant QoS, use the configuration file
             * USER_QOS_PROFILES.xml
             */

            participant = DomainParticipantFactory.TheParticipantFactory
                    .create_participant(domainId,
                            DomainParticipantFactory.PARTICIPANT_QOS_DEFAULT,
                            null /* listener */, StatusKind.STATUS_MASK_NONE);
            if (participant == null) {
                System.err.println("create_participant error\n");
                return;
            }

            /* Start changes for Custom_Flowcontroller */

            /*
             * If you want to change the Participant's QoS programmatically
             * rather than using the XML file, you will need to add the
             * following lines to your code and comment out the
             * create_participant call above.
             */

            /*
             * By default, discovery will communicate via shared memory for
             * platforms that support it. Because we disable shared memory on
             * the publishing side, we do so here to ensure the reader and
             * writer discover each other.
             */

            /* Get default participant QoS to customize */
/*
            DomainParticipantQos participant_qos = new DomainParticipantQos();
            DomainParticipantFactory.TheParticipantFactory
                    .get_default_participant_qos(participant_qos);

            // By default, data will be sent via shared memory _and_ UDPv4.
            // Because the flowcontroller limits writes across all interfaces,
            // this halves the effective send rate. To avoid this, we enable
            // only the UDPv4 transport
            participant_qos.transport_builtin.mask = TransportBuiltinKind.UDPv4;

            // To create participant with default QoS, use
            // DomainParticipantFactory.PARTICIPANT_QOS_DEFAULT
            // instead of participant_qos
            participant = DomainParticipantFactory.TheParticipantFactory
                    .create_participant(domainId, participant_qos, null,
                            StatusKind.STATUS_MASK_NONE);
*/
            /* End changes for Custom_Flowcontroller */

            // --- Create subscriber --- //

            /*
             * To customize subscriber QoS, use the configuration file
             * USER_QOS_PROFILES.xml
             */

            subscriber = participant.create_subscriber(
                    DomainParticipant.SUBSCRIBER_QOS_DEFAULT,
                    null /* listener */, StatusKind.STATUS_MASK_NONE);
            if (subscriber == null) {
                System.err.println("create_subscriber error\n");
                return;
            }

            // --- Create topic --- //

            /* Register type before creating topic */
            String typeName = cfcTypeSupport.get_type_name();
            cfcTypeSupport.register_type(participant, typeName);

            /*
             * To customize topic QoS, use the configuration file
             * USER_QOS_PROFILES.xml
             */

            topic = participant.create_topic("Example cfc", typeName,
                    DomainParticipant.TOPIC_QOS_DEFAULT, null /* listener */,
                    StatusKind.STATUS_MASK_NONE);
            if (topic == null) {
                System.err.println("create_topic error\n");
                return;
            }

            // --- Create reader --- //

            listener = new cfcListener();

            /*
             * To customize data reader QoS, use the configuration file
             * USER_QOS_PROFILES.xml
             */

            reader = (cfcDataReader) subscriber.create_datareader(topic,
                    Subscriber.DATAREADER_QOS_DEFAULT, listener,
                    StatusKind.STATUS_MASK_ALL);
            if (reader == null) {
                System.err.println("create_datareader error\n");
                return;
            }

            // --- Wait for data --- //

            final long receivePeriodSec = 4;

            for (int count = 0; (sampleCount == 0) || (count < sampleCount); 
                    ++count) {

                try {
                    Thread.sleep(receivePeriodSec * 1000); // in millisec
                } catch (InterruptedException ix) {
                    System.err.println("INTERRUPTED");
                    break;
                }
            }
        } finally {

            // --- Shutdown --- //

            if (participant != null) {
                participant.delete_contained_entities();

                DomainParticipantFactory.TheParticipantFactory
                        .delete_participant(participant);
            }
            /*
             * RTI Connext provides the finalize_instance() method for users who
             * want to release memory used by the participant factory singleton.
             * Uncomment the following block of code for clean destruction of
             * the participant factory singleton.
             */
            // DomainParticipantFactory.finalize_instance();
        }
    }

    // -----------------------------------------------------------------------
    // Private Types
    // -----------------------------------------------------------------------

    // =======================================================================

    private static class cfcListener extends DataReaderAdapter {

        cfcSeq _dataSeq = new cfcSeq();
        SampleInfoSeq _infoSeq = new SampleInfoSeq();

        /* Start changes for Custom_Flowcontroller */
        long initms = System.currentTimeMillis();
        DecimalFormat decFormat = new DecimalFormat("#.##");

        public void on_data_available(DataReader reader) {
            cfcDataReader cfcReader = (cfcDataReader) reader;

            try {
                cfcReader.take(_dataSeq, _infoSeq,
                        ResourceLimitsQosPolicy.LENGTH_UNLIMITED,
                        SampleStateKind.ANY_SAMPLE_STATE,
                        ViewStateKind.ANY_VIEW_STATE,
                        InstanceStateKind.ANY_INSTANCE_STATE);

                for (int i = 0; i < _dataSeq.size(); ++i) {
                    SampleInfo info = (SampleInfo) _infoSeq.get(i);
                    if (info.valid_data) {
                        double elapsed = 
                                (System.currentTimeMillis() - initms) / 1000.0;
                        System.out.println("@ t=" + decFormat.format(elapsed)
                                + ", got x = " + ((cfc) _dataSeq.get(i)).x);

                    }
                }
            } catch (RETCODE_NO_DATA noData) {
                // No data to process
            } finally {
                cfcReader.return_loan(_dataSeq, _infoSeq);
            }
        }
    }
}
