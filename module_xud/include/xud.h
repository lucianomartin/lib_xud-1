/*
 * \brief     User defines and functions for XMOS USB Device Layer 
 */

#ifndef __xud_h__
#define __xud_h__

#include <print.h>
#include <xs1.h>

/**
 * \var     typedef XUD_EpType
 * \brief   Typedef for endpoint types.  Note: it is important that ISO is 0
 */
typedef enum XUD_EpType
{
    XUD_EPTYPE_ISO = 0,          /**< Isoc */
    XUD_EPTYPE_INT,              /**< Interrupt */
    XUD_EPTYPE_BUL,              /**< Bulk */
    XUD_EPTYPE_CTL,              /**< Control */
    XUD_EPTYPE_DIS,              /**< Disabled */
} XUD_EpType;

/**
 * \var     typedef XUD_ep
 * \brief   Typedef for endpoint identifiers
 */
typedef unsigned int XUD_ep;

/* Value to be or'ed in with EP type to enable bus state notifications */
#define XUD_STATUS_ENABLE           0x80000000                   

/* Bus state defines */
#define XUD_SPEED_FS                1
#define XUD_SPEED_HS                2

#define XUD_SUSPEND                 3

/* Control token defines - used to inform EPs of bus-state types */
#define USB_RESET_TOKEN             8        /* Control token value that signals RESET */
#define USB_SUSPEND_TOKEN           9        /* Control token value that signals SUSPEND */


/**********************************************************************************************
 * Below are prototypes for main assembly functions for data transfer to/from USB I/O thread 
 * All other Get/Set functions defined here use these.  These are implemented in XUD_EpFuncs.S
 * Wrapper functions are provided for conveniance (implemented in XUD_EpFunctions.xc).  
 */

/**
 *  \brief      Gets a data from XUD
 *  \param      ep_out     The OUT endpoint identifier
 *  \param      buffer     The buffer to store received data into
 *  \return     Datalength (in bytes) 
 */
inline int XUD_GetData(XUD_ep ep_out, unsigned char buffer[]);

/**
 *  \brief      Gets a data from XUD
 *  \param      ep_out     The OUT endpoint identifier
 *  \param      ep_in      The IN endpoint identifier
 *  \param      buffer     The buffer to store received data into
 *  \return     Datalength (in bytes) 
 *  TODO:       Use generic GetData from this 
 */
int XUD_GetSetupData(XUD_ep ep_out, XUD_ep ep_in, unsigned char buffer[]); 

/**
 *  \brief      TBD
 *  \param      ep_in      The IN endpoint identifier
 *  \param      buffer     The packet buffer to send data from
 *  \param      datalength The length of the packet to send (in bytes)
 *  \param      startIndex The start index of the packet in the buffer (typically 0)
 *  \param      pidToggle  Normal usage is 0 causing XUD to toggle packet ID normally. Anything other than 0 is used as the packet ID.
 *  \return                0 on non-error, -1 on bus-reset
 */
int XUD_SetData(XUD_ep ep_in, unsigned char buffer[], unsigned datalength, unsigned startIndex, unsigned pidToggle);

/***********************************************************************************************/

    
/** This performs the low level USB I/O operations. Note that this
 *  needs to run in a thread with at least 80 MIPS worst case execution
 *  speed.
 * 
 *    \param  c_epOut   An array of channel ends, one channel end per 
 *                      output endpoint (USB OUT transaction); this includes
 *                      a channel to obtain requests on Endpoint 0.
 *    \param  noEpOut    The number of output endpoints, should
 *                      be at least 1 (for Endpoint 0).
 *    \param  c_epIn    An array of channel ends, one channel end
 *                      per input endpoint (USB IN transaction); this
 *                      includes a channel to respond to
 *                      requests on Endpoint 0.
 *    \param  noEpIn The number of input endpoints, should be 
 *                  at least 1 (for Endpoint 0).
 *    \param  c_sof   A channel to receive SOF tokens on. This channel
 *                   must be connected to a process that
 *                   can receive a token once every 125 ms. If
 *                   tokens are not read, the USB layer will lock up.
 *                   If no SOF tokens are required ``null`` 
 *                   should be used as this channel.
 *
 *    \param  epTypeTableOut See ``epTypeTableIn``
 *    \param  epTypeTableIn This and ``epTypeTableOut`` are two arrays
 *                            indicating the type of channel ends. 
 *                            Legal types include: 
 *                           ``XUD_EPTYPE_CTL`` (Endpoint 0), 
 *                           ``XUD_EPTYPE_BUL`` (Bulk endpoint),
 *                           ``XUD_EPTYPE_ISO`` (Isochronous endpoint),
 *                           ``XUD_EPTYPE_DIS`` (Endpoint not used).
 *                            The first array contains the
 *                            endpoint types for each of the OUT
 *                            endpoints, the second array contains the
 *                            endpoint types for each of the IN
 *                            endpoints.
 *    \param  p_usb_rst The port to send reset signals to.
 *    \param  clk The clock block to use for the USB reset - 
 *               this should not be clock block 0.
 *    \param  rstMask   The mask to use when taking an external phy into/out of reset. The mask is
 *                      ORed into the port to disable reset, and unset when
 *                      deasserting reset. Use '-1' as a default mask if this
 *                      port is not shared.
 *    \param  desiredSpeed This parameter specifies whether the
 *                         device must be full-speed (ie, USB-1.0) or
 *                         whether high-speed is acceptable if supported
 *                         by the host (ie, USB-2.0). Pass ``XUD_SPEED_HS``
 *                         if high-speed is allowed, and ``XUD_SPEED_FS``
 *                         if not. Low speed USB is not supported by XUD.
 *    \param  c_usb_testmode See :ref:`xud_usb_test_modes`
 *
 */
int XUD_Manager(chanend c_epOut[], int noEpOut, 
                chanend c_epIn[], int noEpIn,
                chanend ?c_sof,
                XUD_EpType epTypeTableOut[], XUD_EpType epTypeTableIn[],
                out port ?p_usb_rst, clock ?clk, unsigned rstMask, unsigned desiredSpeed,
                chanend ?c_usb_testmode);


/**
 * \brief  This function must be called by a thread that deals with an OUT endpoint.
 *         When the host sends data, the low level driver will fill the buffer. It
 *         pauses until data is available.
 * \param  ep_out   The OUT endpoint identifier
 * \param  buffer   The buffer to store data in. This is a buffer of integers, containing
 *                  characters; the buffer must be word aligned.
 * \return The number of bytes written to the buffer (Also see Status Reporting).
 **/
int XUD_GetBuffer(XUD_ep ep_out, unsigned char buffer[]);


/**
 * \brief  Request setup data from usb buffer for a specific endpoint, pauses until data is available.  
 * \param  ep_out   The OUT endpoint identifier
 * \param  ep_in    The IN endpoint identifier
 * \param  buffer   A char buffer passed by ref into which data is returned
 * \return datalength in bytes (always 8)
 **/
int XUD_GetSetupBuffer(XUD_ep ep_out, XUD_ep ep_in, unsigned char buffer[]);


/**
 * \brief  This function must be called by a thread that deals with an IN endpoint.
 *         When the host asks for data, the low level driver will transmit the buffer
 *         to the host.
 * \param  ep_in The endpoint identifier created by ``XUD_Init_Ep``
 * \param  buffer The buffer of data to send out.  
 * \param  datalength The number of bytes in the buffer.
 * \return TBD
 */
int XUD_SetBuffer(XUD_ep ep_in, unsigned char buffer[], unsigned datalength);


/* Same as above but takes a max packet size for the endpoint, breaks up data to transfers of no 
 * greater than this.
 *
 * NOTE: This function reasonably assumes the max transfer size for an endpoint is word aligned  
 **/

/**
 * \brief   Similar to XUD_SetBuffer but breaks up data transfers of into smaller packets.
 *          This function must be called by a thread that deals with an IN endpoint.
 *          When the host asks for data, the low level driver will transmit the buffer
 *          to the host.  
 * \param   ep_in        The IN endpoint identifier created by ``XUD_Init_Ep``
 * \param   buffer       The buffer of data to send out.  
 * \param   datalength   The number of bytes in the buffer.
 * \param   epMax        The maximum packet size in bytes
 * \return  0 on success, for errors see :ref:`xud_status_reporting`
 */
int XUD_SetBuffer_EpMax(XUD_ep ep_in, unsigned char buffer[], unsigned datalength, unsigned epMax);


/**
 * \brief  This function performs a combined ``XUD_SetBuffer`` and ``XUD_GetBuffer``.
 *         It transmits the buffer of the given length over the ``ep_in`` channel to 
 *         answer an IN request, and then waits for an OUT transaction on ``ep_out``.
 *         This function is normally called to handle Get control requests to endpoint 0.
 * 
 * \param  ep_out The endpoint identifier that handles endpoint 0 OUT data in the XUD manager.
 * \param  ep_in The endpoint identifier that handles endpoint 0 IN data in the XUD manager.
 * \param  buffer The data to send in response to the IN transaction. Note that this data
 *         is chopped up in fragments of at most 64 bytes.
 * \param  length Length of data to be sent
 * \param  requested  The length that the host requested, pass the value ``sp.wLength``.
 * 
 * \return 0 on success, for errors see :ref:`xud_status_reporting`
 **/
int XUD_DoGetRequest(XUD_ep ep_out, XUD_ep ep_in,  unsigned char buffer[], unsigned length, unsigned requested);


/**
 * \brief  This function sends an empty packet back on the next IN request with
 *         PID1. It is normally used by Endpoint 0 to acknowledge success of a control transfer.
 * \param  ep_in The endpoint 0 IN identifier to the XUD manager.
 * 
 * \return 0 on success, for errors see :ref:`xud_status_reporting`
 **/
int XUD_DoSetRequestStatus(XUD_ep ep_in);


/**
 * \brief  This function must be called by endpoint 0 once a ``setDeviceAddress``
 *         request is made by the host.
 * \param  addr New device address
 * \warning Must be run on USB core
 */
void XUD_SetDevAddr(unsigned addr);


/**
 * \brief  This function will complete a reset on an endpoint. One can either pass
 *         one or two channel-ends in (the second channel-end can be set to ``null``).
 *         The return value should be inspected to find out what type of reset was
 *         performed. In endpoint 0 typically two channels are reset (IN and OUT).
 *         In other endpoints ``null`` can be passed as the second parameter.
 * \param  one IN or OUT endpoint identifier to perform the reset on.
 * \param  two Optional second IN or OUT endpoint structure to perform a reset on.
 * \return One of: ``XUD_SPEED_HS`` The host has accepted that this device can execute
 *         at high speed. ``XUD_SPEED_FS`` The device should run at full speed.
 */
int XUD_ResetEndpoint(XUD_ep one, XUD_ep &?two);


/**
 * \brief  Initialises an XUD_ep
 * \param  c_ep Endpoint channel to be connected to the XUD library.
 * \return Endpoint descriptor
 */
XUD_ep XUD_InitEp(chanend c_ep);


/**
 * \brief   Mark an OUT endpoint as STALL.  Note: is cleared automatically if a SETUP received on endpoint
 * \param   epNum Endpoint number
 * \return  void
 * \warning Must be run on USB core
 */
void XUD_SetStall_Out(int epNum);


/**
 * \brief   Mark an IN endpoint as STALL.  Note: is cleared automatically if a SETUP received on endpoint
 * \param   epNum Endpoint number
 * \return  void
 * \warning Must be run on USB core
 */
void XUD_SetStall_In(int epNum);


/**
 * \brief   Mark an OUT endpoint as NOT STALLed.
 * \param   epNum Endpoint number
 * \return  void
 * \warning Must be run on USB core
 */
void XUD_ClearStall_Out(int epNum);


/**
 * \brief   Mark an IN endpoint as NOT STALLed.
 * \param   epNum Endpoint number
 * \return  void
 * \warning Must be run on USB core
 */
void XUD_ClearStall_In(int epNum);


/* Advanced functions for supporting multple Endpoints in a single core */


/**
 * \brief   TBD
 */
#pragma select handler
void XUD_GetData_Select(chanend c, XUD_ep ep, int &tmp);

/**
 * \brief   TBD
 */
#pragma select handler
void XUD_SetData_Select(chanend c, XUD_ep ep, int &tmp);

/**
 * \brief   TBD
 */
inline void XUD_SetReady_Out(XUD_ep e, unsigned char bufferPtr[])
{
    int chan_array_ptr;
    asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(e));
    asm ("stw %0, %1[3]"::"r"(bufferPtr),"r"(e));            // Store buffer 
    asm ("stw %0, %1[0]"::"r"(e),"r"(chan_array_ptr));            
  
}

/**
 * \brief   TBD
 */
inline void XUD_SetReady_OutPtr(XUD_ep ep, unsigned addr)
{
    int chan_array_ptr;
    
    asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(ep));
    asm ("stw %0, %1[3]"::"r"(addr),"r"(ep));            // Store buffer 
    asm ("stw %0, %1[0]"::"r"(ep),"r"(chan_array_ptr));        
}

/**
 * \brief   TBD
 */
inline void XUD_SetReady_In(XUD_ep e, unsigned char bufferPtr[], int len)
{
    int chan_array_ptr;
    int tmp, tmp2;
    int wordlength;
    int taillength;

    /* Knock off the tail bits */
    wordlength = len >>2;
    wordlength <<=2;

    taillength = zext((len << 5),7);

    asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(e));
    
    // Get end off buffer address
    asm ("add %0, %1, %2":"=r"(tmp):"r"(bufferPtr),"r"(wordlength));             

    asm ("neg %0, %1":"=r"(tmp2):"r"(len>>2));            // Produce negative offset from end off buffer

    // Store neg index 
    asm ("stw %0, %1[6]"::"r"(tmp2),"r"(e));            // Store index 
    
    // Store buffer pointer
    asm ("stw %0, %1[3]"::"r"(tmp),"r"(e));             

    // Store tail len
    asm ("stw %0, %1[7]"::"r"(taillength),"r"(e));             


    asm ("stw %0, %1[0]"::"r"(e),"r"(chan_array_ptr));      // Mark ready 

}

/**
 * \brief   TBD
 */
inline void XUD_SetReady_InPtr(XUD_ep ep, unsigned addr, int len)
{
    int chan_array_ptr;
    int tmp, tmp2;
    int wordlength;
    int taillength;

    /* Knock off the tail bits */
    wordlength = len >>2;
    wordlength <<=2;

    taillength = zext((len << 5),7);

    asm ("ldw %0, %1[0]":"=r"(chan_array_ptr):"r"(ep));
    
    // Get end off buffer address
    asm ("add %0, %1, %2":"=r"(tmp):"r"(addr),"r"(wordlength));             

    asm ("neg %0, %1":"=r"(tmp2):"r"(len>>2));            // Produce negative offset from end off buffer

    // Store neg index 
    asm ("stw %0, %1[6]"::"r"(tmp2),"r"(ep));            // Store index 
    
    // Store buffer poinr
    asm ("stw %0, %1[3]"::"r"(tmp),"r"(ep));             

    // Store tail len
    asm ("stw %0, %1[7]"::"r"(taillength),"r"(ep));             

    asm ("stw %0, %1[0]"::"r"(ep),"r"(chan_array_ptr));      // Mark ready 

}

/**
 *  \brief      TBD
 */
int XUD_ResetDrain(chanend one);

/**
 *  \brief      TBD
 */
int XUD_GetBusSpeed(chanend c);

#endif // __xud_h__
