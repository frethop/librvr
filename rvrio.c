#include "rvrio.h"

int serial_port_fd;
int logging_level;
int sequence_number;

/*
 * Used stackoverflow for this, because it was easy:
 * https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
 */

 int
set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                fprintf (stderr, "error %d from tcgetattr", errno);
                return -1;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                        // enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
                fprintf (stderr, "error %d from tcsetattr", errno);
                return -1;
        }
        return 0;
}

void
set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
                fprintf (stderr, "error %d from tggetattr", errno);
                return;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
                fprintf (stderr, "error %d setting term attributes", errno);
}

/***********************************************************************************/

void init_port() {
    
    char *portname = "/dev/ttyS0";
    serial_port_fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
    if (serial_port_fd < 0) {
        fprintf (stderr, "error %d opening %s: %s", errno, portname, strerror (errno));
        return;
    }

    set_interface_attribs (serial_port_fd, B115200, 0);  // set speed to 115,200 bps, 8n1 (no parity)
    set_blocking (serial_port_fd, 0); 

    sequence_number = 1;
    logging_level = 0;
}       

void set_logging_level(int level) {
    logging_level = level;
}

void log_flags(struct header * headr) {
    printf("\n");
    printf("%1d.......: %s\n", headr->flags.flag_bits.is_response, 
                               (headr->flags.flag_bits.is_response==0?"no response":"is a response"));
    printf(".%1d......: %s\n", headr->flags.flag_bits.response_requested, 
                               (headr->flags.flag_bits.response_requested==0?"no response requested":"response requested"));
    printf("..%1d.....: %s\n", headr->flags.flag_bits.response_requested_if_error, 
                               (headr->flags.flag_bits.response_requested_if_error==0?"no error response requested":"error response requested"));
    printf("...%1d....: %s\n", headr->flags.flag_bits.is_activity,
                               (headr->flags.flag_bits.is_activity==0?"is not an activity":"is an activity"));
    printf("....%1d...: %s\n", headr->flags.flag_bits.has_target, 
                               (headr->flags.flag_bits.has_target==0?"no target":"has target"));
    printf(".....%1d..: %s\n", headr->flags.flag_bits.has_source, 
                               (headr->flags.flag_bits.has_source==0?"no source":"has source"));
    printf("......0.: %s\n", "UNUSED");
    printf(".......%1d: %s\n", headr->flags.flag_bits.has_more_flags, 
                               (headr->flags.flag_bits.has_more_flags==0?"no more flags":"has more flags"));
}

struct message * read_message(int recv_length) {

    u_int8_t byte;
    int checksum = 0;

    struct message * msg = (struct message*) malloc(sizeof(struct message));
    struct header * headr = (struct header *) malloc(sizeof (struct header));
    msg->msghdr = headr;

    /* Start the message */
    read(serial_port_fd, &byte, 1);
    if (logging_level >= BYTESINFO) printf("READ: [%02x ", byte);

    /* Flags */
    read(serial_port_fd, &byte, 1);
    msg->msghdr->flags.allbits = byte;
    checksum += byte;
    if (logging_level >= BYTESINFO) printf("%02x ", byte);
    if (logging_level >= VERBOSE) log_flags(msg->msghdr);

    /* Rest of the header */
    if (msg->msghdr->flags.flag_bits.has_target == 1) {
        read(serial_port_fd, &byte, 1);
        msg->msghdr->targetID.wholeID = byte;;
        checksum += byte;
        if (logging_level >= BYTESINFO) printf("%02x ", byte);
    }

    if (msg->msghdr->flags.flag_bits.has_source == 1) {
        read(serial_port_fd, &byte, 1);
        msg->msghdr->sourceID.wholeID = byte;;
        checksum += byte;
        if (logging_level >= BYTESINFO) printf("%02x ", byte);
    }

    read(serial_port_fd, &byte, 1);
    msg->msghdr->deviceID = byte;
    checksum += byte;
    if (logging_level >= BYTESINFO) printf("%02x ", byte);
 
    read(serial_port_fd, &byte, 1);
    msg->msghdr->commandID = byte;
    checksum += byte;
    if (logging_level >= BYTESINFO) printf("%02x ", byte);

    read(serial_port_fd, &byte, 1);
    msg->msghdr->sequence_num = byte;
    checksum += byte;
    if (logging_level >= BYTESINFO) printf("%02x ", byte);

    /* And now the payload */
    if (recv_length > 0) {
        u_int8_t * pl = (u_int8_t *) malloc(sizeof(u_int8_t)*recv_length);
        msg->payload = pl;
        if (logging_level >= BYTESINFO) printf("{");
        for (int i=0; i < recv_length; i++) {
            read(serial_port_fd, &byte, 1);
            *pl = byte;
            checksum += byte;
            if (logging_level >= BYTESINFO) printf("%02x ", byte);
            pl++;
        }
        if (logging_level >= BYTESINFO) printf("} ");
    }

    /* Checksum */
    checksum = checksum & 0xFF ^ 0xFF;
    read(serial_port_fd, &byte, 1);
    if (byte != checksum) {
    }
    if (logging_level >= BYTESINFO) printf("%02x ", byte);
    
    /* Message ends */
    read(serial_port_fd, &byte, 1);
    while (byte != MESSAGE_END) {
       if (logging_level >= BYTESINFO) printf("%02x* ", byte);   
       read(serial_port_fd, &byte, 1);
    }
    if (logging_level >= BYTESINFO) printf("%02x]\n", byte);

    return msg;
}

void write_message(struct message * msg) {

    u_int8_t byte;
    int checksum = 0;

    /* Start the message */
    byte = MESSAGE_START;
    write(serial_port_fd, &byte, 1);
    if (logging_level >= BYTESINFO) printf("WRITE: [%02x ", byte);

    /* Flags */
    byte = msg->msghdr->flags.allbits;
    write(serial_port_fd, &byte, 1);
    checksum += byte;
    if (logging_level >= BYTESINFO) printf("%02x ", byte);
    if (logging_level >= VERBOSE) log_flags(msg->msghdr);

    /* Rest of the header */
    if (msg->msghdr->flags.flag_bits.has_target == 1) {
        byte = msg->msghdr->targetID.wholeID;
        write(serial_port_fd, &byte, 1);
        checksum += byte;
        if (logging_level >= BYTESINFO) printf("%02x ", byte);
    }

    if (msg->msghdr->flags.flag_bits.has_source == 1) {
        byte = msg->msghdr->sourceID.wholeID;
        write(serial_port_fd, &byte, 1);
        checksum += byte;
        if (logging_level >= BYTESINFO) printf("%02x ", byte);
    }

    byte = msg->msghdr->deviceID;
    write(serial_port_fd, &byte, 1);
    checksum += byte;
    if (logging_level >= BYTESINFO) printf("%02x ", byte);
 
    byte = msg->msghdr->commandID;
    write(serial_port_fd, &byte, 1);
    checksum += byte;
    if (logging_level >= BYTESINFO) printf("%02x ", byte);

    byte = msg->msghdr->sequence_num;
    write(serial_port_fd, &byte, 1);
    checksum += byte;
    if (logging_level >= BYTESINFO) printf("%02x ", byte);

    /* And now the payload */
    if (msg->loadlength > 0) {
        u_int8_t * pl = msg->payload;
        if (logging_level >= BYTESINFO) printf("{");
        for (int i=0; i < msg->loadlength; i++) {
            byte = *pl;
            write(serial_port_fd, &byte, 1);
            checksum += byte;
            if (logging_level >= BYTESINFO) printf("%02x ", byte);
            pl++;
        }
        if (logging_level >= BYTESINFO) printf("} ");
    }

    /* Checksum */
    byte = checksum & 0xFF ^ 0xFF;
    write(serial_port_fd, &byte, 1);
    if (logging_level >= BYTESINFO) printf("%02x ", byte);
    
    /* Message ends */
    byte = MESSAGE_END;
    write(serial_port_fd, &byte, 1);
    if (logging_level >= BYTESINFO) printf("%02x]\n", byte);
}

void messageSend(u_int8_t cid, u_int8_t did, 
                 u_int8_t source, u_int8_t target, 
                 u_int8_t * payload, u_int8_t payload_length) {

    /* Build the header */
    struct header * headr = (struct header *) malloc(sizeof (struct header));

    /* Flags */
    headr->flags.allbits = 0;
    headr->flags.flag_bits.is_activity = 1;
    headr->flags.flag_bits.has_target = (target > 0 ? 1 : 0);
    headr->flags.flag_bits.has_source = (source > 0 ? 1 : 0);
    
    /* Rest of the header */
    headr->targetID.wholeID = target;
    headr->sourceID.wholeID = source;
    headr->deviceID = did;
    headr->commandID = cid;
    headr->sequence_num = sequence_number++;

    /* Build the message */
    struct message * msg = (struct message*) malloc(sizeof(struct message));

    msg->msghdr = headr;
    msg->payload = payload;
    msg->loadlength = payload_length;

    /* Write it! */
    write_message(msg);
}

u_int8_t * messageSendAndRecv(u_int8_t cid, u_int8_t did, 
                              u_int8_t source, u_int8_t target, 
                              u_int8_t * payload, u_int8_t payload_length, u_int8_t recv_length) {

     /* Build the header */
    struct header * headr = (struct header *) malloc(sizeof (struct header));

    /* Flags */
    headr->flags.allbits = 0;
    headr->flags.flag_bits.is_activity = 1;
    headr->flags.flag_bits.response_requested = 1;
    headr->flags.flag_bits.has_target = (target > 0 ? 1 : 0);
    headr->flags.flag_bits.has_source = (source > 0 ? 1 : 0);
    
    /* Rest of the header */
    headr->targetID.wholeID = target;
    headr->sourceID.wholeID = source;
    headr->deviceID = did;
    headr->commandID = cid;
    headr->sequence_num = sequence_number++;

    /* Build the message */
    struct message * msg = (struct message*) malloc(sizeof(struct message));

    msg->msghdr = headr;
    msg->payload = payload;
    msg->loadlength = payload_length;

    /* Write it! */
    write_message(msg);

    /* and read it! */
    struct message * response = read_message(recv_length);

    return response->payload;
}

