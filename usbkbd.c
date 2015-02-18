/*
 *  Copyright (c) 1999-2001 Vojtech Pavlik
 *
 *  USB HIDBP Keyboard support
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Should you need to contact me, the author, you can do so either by
 * e-mail - mail your message to <vojtech@ucw.cz>, or by paper mail:
 * Vojtech Pavlik, Simunkova 1594, Prague 8, 182 00 Czech Republic
 

 * This code is originally taken from linux distribution and modified
 * by Azizul Hakim for beaglebone to be compatible with AOA supported
 * android devices.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>

/*
 * Version Information
 */
#define DRIVER_VERSION ""
#define DRIVER_AUTHOR "Azizul Hakim <azizulfahim2002@gmail.com>"
#define DRIVER_DESC "USB HID Boot Protocol keyboard driver"
#define DRIVER_LICENSE "GPL"

#define REQ_GET_PROTOCOL				51
#define REQ_SEND_ID						52
#define REQ_AOA_ACTIVATE				53
#define ACCESSORY_REGISTER_HID			54
#define ACCESSORY_UNREGISTER_HID		55
#define ACCESSORY_SET_HID_REPORT_DESC	56
#define ACCESSORY_SEND_HID_EVENT		57
#define ACCESSORY_REGISTER_AUDIO		58

#define MANU	"IsonProjects"
#define MODEL	"BeagleDroid"
#define DESCRIPTION	"BEAGLEBOARD-ANDROID KEYBOARD MONITOR DRIVER"
#define VERSION	"1.0"
#define URI		"www.google.com"
#define SERIAL	"10"

#define ID_MANU		0
#define ID_MODEL	1
#define ID_DESC		2
#define ID_VERSION	3
#define ID_URI		4
#define ID_SERIAL	5

#define VAL_AOA_REQ		0
#define VAL_HID_MOUSE	1234
#define VAL_NO_AUDIO	0
#define VAL_AUDIO		1

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);

static const unsigned char usb_kbd_keycode[256] = {
	  0,  0,  0,  0, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38,
	 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44,  2,  3,
	  4,  5,  6,  7,  8,  9, 10, 11, 28,  1, 14, 15, 57, 12, 13, 26,
	 27, 43, 43, 39, 40, 41, 51, 52, 53, 58, 59, 60, 61, 62, 63, 64,
	 65, 66, 67, 68, 87, 88, 99, 70,119,110,102,104,111,107,109,106,
	105,108,103, 69, 98, 55, 74, 78, 96, 79, 80, 81, 75, 76, 77, 71,
	 72, 73, 82, 83, 86,127,116,117,183,184,185,186,187,188,189,190,
	191,192,193,194,134,138,130,132,128,129,131,137,133,135,136,113,
	115,114,  0,  0,  0,121,  0, 89, 93,124, 92, 94, 95,  0,  0,  0,
	122,123, 90, 91, 85,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 29, 42, 56,125, 97, 54,100,126,164,166,165,163,161,115,114,113,
	150,158,159,128,136,177,178,176,142,152,173,140
};

static const unsigned char usb_hid_mouse_report_desc[50] = 
				{	0x05, 0x01,                    /* USAGE_PAGE (Generic Desktop) */ 
					0x09, 0x02,                    /* USAGE (Mouse) */ 
					0xa1, 0x01,                    /* COLLECTION (Application) */ 
					0x09, 0x01,                    /*   USAGE (Pointer) */ 
					0xa1, 0x00,                    /*   COLLECTION (Physical) */ 
					0x05, 0x09,                    /*     USAGE_PAGE (Button) */ 
					0x19, 0x01,                    /*     USAGE_MINIMUM (Button 1) */ 
					0x29, 0x03,                    /*     USAGE_MAXIMUM (Button 3) */ 
					0x15, 0x00,                    /*     LOGICAL_MINIMUM (0) */ 
					0x25, 0x01,                    /*     LOGICAL_MAXIMUM (1) */ 
					0x95, 0x03,                    /*     REPORT_COUNT (3) */ 
					0x75, 0x01,                    /*     REPORT_SIZE (1) */ 
					0x81, 0x02,                    /*     INPUT (Data,Var,Abs) */ 
					0x95, 0x01,                    /*     REPORT_COUNT (1) */ 
					0x75, 0x05,                    /*     REPORT_SIZE (5) */ 
					0x81, 0x03,                    /*     INPUT (Cnst,Var,Abs) */ 
					0x05, 0x01,                    /*     USAGE_PAGE (Generic Desktop) */ 
					0x09, 0x30,                    /*     USAGE (X) */ 
					0x09, 0x31,                    /*     USAGE (Y) */ 
					0x15, 0x81,                    /*     LOGICAL_MINIMUM (-127) */ 
					0x25, 0x7f,                    /*     LOGICAL_MAXIMUM (127) */ 
					0x75, 0x08,                    /*     REPORT_SIZE (8) */ 
					0x95, 0x02,                    /*     REPORT_COUNT (2) */ 
					0x81, 0x06,                    /*     INPUT (Data,Var,Rel) */ 
					0xc0,                          /*   END_COLLECTION */ 
					0xc0                           /* END_COLLECTION */ 
				};


/**
 * struct usb_kbd - state of each attached keyboard
 * @dev:	input device associated with this keyboard
 * @usbdev:	usb device associated with this keyboard
 * @old:	data received in the past from the @irq URB representing which
 *		keys were pressed. By comparing with the current list of keys
 *		that are pressed, we are able to see key releases.
 * @irq:	URB for receiving a list of keys that are pressed when a
 *		new key is pressed or a key that was pressed is released.
 * @led:	URB for sending LEDs (e.g. numlock, ...)
 * @newleds:	data that will be sent with the @led URB representing which LEDs
 		should be on
 * @name:	Name of the keyboard. @dev's name field points to this buffer
 * @phys:	Physical path of the keyboard. @dev's phys field points to this
 *		buffer
 * @new:	Buffer for the @irq URB
 * @cr:		Control request for @led URB
 * @leds:	Buffer for the @led URB
 * @new_dma:	DMA address for @irq URB
 * @leds_dma:	DMA address for @led URB
 * @leds_lock:	spinlock that protects @leds, @newleds, and @led_urb_submitted
 * @led_urb_submitted: indicates whether @led is in progress, i.e. it has been
 *		submitted and its completion handler has not returned yet
 *		without	resubmitting @led
 */
struct usb_kbd {
	struct input_dev *dev;
	struct usb_device *usbdev;
	unsigned char old[8];
	struct urb *irq, *led;
	unsigned char newleds;
	char name[128];
	char phys[64];

	unsigned char *new;
	struct usb_ctrlrequest *cr;
	unsigned char *leds;
	dma_addr_t new_dma;
	dma_addr_t leds_dma;
	
	spinlock_t leds_lock;
	bool led_urb_submitted;

};

char* utf8(const char *str)
{
	char *utf8;
	utf8 = kmalloc(1+(2*strlen(str)), GFP_KERNEL);

	if (utf8) {
		char *c = utf8;
		for (; *str; ++str) {
			if (*str & 0x80) {
				*c++ = *str;
			} else {
				*c++ = (char) (0xc0 | (unsigned) *str >> 6);
				*c++ = (char) (0x80 | (*str & 0x3f));
			}
		}
		*c++ = '\0';
	}
	return utf8;
}

int GetProtocol(struct usb_device *usbdev, char *buffer){
	//Debug_Print("BEAGLEDROID-KBD", "Get Protocol: AOA version");
	
	return usb_control_msg(usbdev,
			usb_rcvctrlpipe(usbdev, 0),
			REQ_GET_PROTOCOL,
			USB_DIR_IN | USB_TYPE_VENDOR,
			VAL_AOA_REQ,
			0,
			buffer,
			sizeof(buffer),
			HZ*5);	
}

int SendIdentificationInfo(struct usb_device *usbdev, int id_index, char *id_info){
	int ret = usb_control_msg(usbdev,
				usb_sndctrlpipe(usbdev, 0),
				REQ_SEND_ID,
				USB_DIR_OUT | USB_TYPE_VENDOR,
				VAL_AOA_REQ,
				id_index,
				utf8((char*)(id_info)),
				sizeof(id_info) + 1,
				HZ*5);

	return ret;
}

int SendAOAActivationRequest(struct usb_device *usbdev){
//	Debug_Print("BEAGLEDROID-KBD", "REQ FOR AOA Mode Activation");

	return usb_control_msg(usbdev,
							usb_sndctrlpipe(usbdev, 0),
							REQ_AOA_ACTIVATE,
							USB_DIR_OUT | USB_TYPE_VENDOR,
							VAL_AOA_REQ,
							0,
							NULL,
							0,
							HZ*5);
}

int SendAudioActivationRequest(struct usb_device *usbdev){
	return usb_control_msg(usbdev,
							usb_sndctrlpipe(usbdev, 0),
							ACCESSORY_REGISTER_AUDIO,
							USB_DIR_OUT | USB_TYPE_VENDOR,
							VAL_AUDIO,
							0,
							NULL,
							0,
							HZ*5);
}

int RegisterHID(struct usb_device *usbdev){
//	Debug_Print("BEAGLEDROID-KBD", "REQ FOR AOA Mode Activation");

	return usb_control_msg(usbdev,
							usb_sndctrlpipe(usbdev, 0),
							ACCESSORY_REGISTER_HID,
							USB_DIR_OUT | USB_TYPE_VENDOR,
							VAL_HID_MOUSE,
							sizeof(usb_hid_mouse_report_desc),
							NULL,
							0,
							HZ*5);
}

int SetConfiguration(struct usb_device *usbdev, char *buffer){
	//Debug_Print("BEAGLEDROID-KBD", "Get Protocol: AOA version");
	
	return usb_control_msg(usbdev,
			usb_rcvctrlpipe(usbdev, 0),
			0x9,
			USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
			1,
			0,
			NULL,
			0,
			HZ*5);	
}


static void usb_kbd_irq(struct urb *urb)
{
	struct usb_kbd *kbd = urb->context;
	int i;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		goto resubmit;
	}

	//printk("Bulk data received\n");
//	printk("%d  %d  %d\n", kbd->new[0], kbd->new[1], kbd->new[2]);

	
//input_report_key(kbd->dev, BTN_LEFT,   0x01);
//input_report_key(kbd->dev, BTN_RIGHT,  0x02);

//	input_report_rel(kbd->dev, REL_X,  50);
//    input_report_rel(kbd->dev, REL_Y,  50);

	/*input_report_key(kbd->dev, 0x04,   0x02);
	input_report_key(kbd->dev, 0x05,   0x03);
	input_report_key(kbd->dev, 0x06,   0x01);
	input_report_key(kbd->dev, 0x07,   0x01);
	input_report_key(kbd->dev, 0x07,   0x00);*/

	input_report_key(kbd->dev, 32,   0x01);
	input_report_key(kbd->dev, 32,   0x00);

	/*for (i=0; i<4; i++){
		input_report_key(kbd->dev, usb_kbd_keycode[kbd->new[i]], 1);
	}
	input_report_key(kbd->dev, usb_kbd_keycode[kbd->new[i-1]], 0);
*/

	for (i=0; i<8; i+=2){
		if (kbd->new[i] != 0 || kbd->new[i+1] != 0){
			printk("%d %d\n", (int)kbd->new[i], (int)kbd->new[i+1]);
			input_report_rel(kbd->dev, REL_X, (int)kbd->new[i]);
			input_report_rel(kbd->dev, REL_Y, (int)kbd->new[i + 1]);
			input_sync(kbd->dev);
		}
	}

	/*for (i = 0; i < 8; i++)			// CTRL, SHIFT, ATL, WIN - L/R each
		input_report_key(kbd->dev, usb_kbd_keycode[i + 224], (kbd->new[0] >> i) & 1);*/
/*
	for (i = 2; i < 8; i++) {

		if (kbd->old[i] > 3 && memscan(kbd->new + 2, kbd->old[i], 6) == kbd->new + 8) {
			if (usb_kbd_keycode[kbd->old[i]])
				input_report_key(kbd->dev, usb_kbd_keycode[kbd->old[i]], 0);
			else
				hid_info(urb->dev,
					 "Unknown key (scancode %#x) released.\n",
					 kbd->old[i]);
		}

		if (kbd->new[i] > 3 && memscan(kbd->old + 2, kbd->new[i], 6) == kbd->old + 8) {
			if (usb_kbd_keycode[kbd->new[i]])
				input_report_key(kbd->dev, usb_kbd_keycode[kbd->new[i]], 1);
			else
				hid_info(urb->dev,
					 "Unknown key (scancode %#x) pressed.\n",
					 kbd->new[i]);
		}
	}

	input_sync(kbd->dev);

	memcpy(kbd->old, kbd->new, 8);
*/
resubmit:
	i = usb_submit_urb (urb, GFP_ATOMIC);
	if (i)
		hid_err(urb->dev, "can't resubmit intr, %s-%s/input0, status %d",
			kbd->usbdev->bus->bus_name,
			kbd->usbdev->devpath, i);
}

static int usb_kbd_event(struct input_dev *dev, unsigned int type,
			 unsigned int code, int value)
{
	unsigned long flags;
	struct usb_kbd *kbd = input_get_drvdata(dev);

	if (type != EV_LED)
		return -1;

	spin_lock_irqsave(&kbd->leds_lock, flags);
	kbd->newleds = (!!test_bit(LED_KANA,    dev->led) << 3) | (!!test_bit(LED_COMPOSE, dev->led) << 3) |
		       (!!test_bit(LED_SCROLLL, dev->led) << 2) | (!!test_bit(LED_CAPSL,   dev->led) << 1) |
		       (!!test_bit(LED_NUML,    dev->led));

	if (kbd->led_urb_submitted){
		spin_unlock_irqrestore(&kbd->leds_lock, flags);
		return 0;
	}

	if (*(kbd->leds) == kbd->newleds){
		spin_unlock_irqrestore(&kbd->leds_lock, flags);
		return 0;
	}

	*(kbd->leds) = kbd->newleds;
	
	kbd->led->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->led, GFP_ATOMIC))
		pr_err("usb_submit_urb(leds) failed\n");
	else
		kbd->led_urb_submitted = true;
	
	spin_unlock_irqrestore(&kbd->leds_lock, flags);
	
	return 0;
}

static void usb_kbd_led(struct urb *urb)
{
	unsigned long flags;
	struct usb_kbd *kbd = urb->context;

	if (urb->status)
		hid_warn(urb->dev, "led urb status %d received\n",
			 urb->status);

	spin_lock_irqsave(&kbd->leds_lock, flags);

	if (*(kbd->leds) == kbd->newleds){
		kbd->led_urb_submitted = false;
		spin_unlock_irqrestore(&kbd->leds_lock, flags);
		return;
	}

	*(kbd->leds) = kbd->newleds;
	
	kbd->led->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->led, GFP_ATOMIC)){
		hid_err(urb->dev, "usb_submit_urb(leds) failed\n");
		kbd->led_urb_submitted = false;
	}
	spin_unlock_irqrestore(&kbd->leds_lock, flags);
	
}

static int usb_kbd_open(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	kbd->irq->dev = kbd->usbdev;
	if (usb_submit_urb(kbd->irq, GFP_KERNEL))
		return -EIO;

	return 0;
}

static void usb_kbd_close(struct input_dev *dev)
{
	struct usb_kbd *kbd = input_get_drvdata(dev);

	usb_kill_urb(kbd->irq);
}

static int usb_kbd_alloc_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	if (!(kbd->irq = usb_alloc_urb(0, GFP_KERNEL)))
		return -1;
	if (!(kbd->led = usb_alloc_urb(0, GFP_KERNEL)))
		return -1;
	if (!(kbd->new = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &kbd->new_dma)))
		return -1;
	if (!(kbd->cr = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL)))
		return -1;
	if (!(kbd->leds = usb_alloc_coherent(dev, 1, GFP_ATOMIC, &kbd->leds_dma)))
		return -1;

	return 0;
}

static void usb_kbd_free_mem(struct usb_device *dev, struct usb_kbd *kbd)
{
	usb_free_urb(kbd->irq);
	//usb_free_urb(kbd->led);
	usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
	kfree(kbd->cr);
	//usb_free_coherent(dev, 1, kbd->leds, kbd->leds_dma);
}

static int usb_kbd_probe(struct usb_interface *iface,
			 const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(iface);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_kbd *kbd;
	struct input_dev *input_dev;
	int i, pipe, maxp;
	int error = -ENOMEM;
	u8 data[8];
	int datalen = 0;

	if (id->idVendor == 0x18D1 && (id->idProduct == 0x2D00 || id->idProduct == 0x2D01 ||
									id->idProduct == 0x2D02 || id->idProduct == 0x2D03 ||
									id->idProduct == 0x2D04 || id->idProduct == 0x2D05)){
		printk("BEAGLEDROID-KBD:  [%04X:%04X] Connected in AOA mode\n", id->idVendor, id->idProduct);


		interface = iface->altsetting;
		/*if (iface->altsetting != NULL)
			interface = iface->altsetting;

		if (interface->desc.bNumEndpoints == 0){
			printk("ERROR!!!\n");

			interface = iface->altsetting;
			if (interface->desc.bNumEndpoints == 0){
				printk("paisi tore\n");
			}
			return -ENODEV;
		}*/


		/*printk("IN->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		for (i=0; i<16; i++){
			endpoint = dev->ep_in[i];
			
			if (endpoint != NULL){
				if (usb_endpoint_is_int_in(endpoint))
					printk("INT IN\n");
				if (usb_endpoint_is_bulk_in(endpoint))
					printk("BULK IN\n");
				if (usb_endpoint_is_isoc_in(endpoint))
					printk("ISOC IN\n");
			}

			if (endpoint != NULL){
				if (usb_endpoint_is_int_out(endpoint))
					printk("INT OUT\n");
				if (usb_endpoint_is_bulk_out(endpoint))
					printk("BULK OUT\n");
				if (usb_endpoint_is_isoc_out(endpoint))
					printk("ISOC OUT\n");
			}
		}

		printk("OUT->>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
		for (i=0; i<16; i++){
			endpoint = dev->ep_out[i];
			
			if (endpoint != NULL){
				if (usb_endpoint_is_int_in(endpoint))
					printk("INT IN\n");
				if (usb_endpoint_is_bulk_in(endpoint))
					printk("BULK IN\n");
				if (usb_endpoint_is_isoc_in(endpoint))
					printk("ISOC IN\n");
			}

			if (endpoint != NULL){
				if (usb_endpoint_is_int_out(endpoint))
					printk("INT OUT\n");
				if (usb_endpoint_is_bulk_out(endpoint))
					printk("BULK OUT\n");
				if (usb_endpoint_is_isoc_out(endpoint))
					printk("ISOC OUT\n");
			}
		}*/
		
		printk("Alternate Settings = %d\n", iface->num_altsetting);

		endpoint = &interface->endpoint[0].desc;
		if (usb_endpoint_is_int_in(endpoint))
			printk("INT IN\n");
		if (usb_endpoint_is_bulk_in(endpoint))
			printk("BULK IN\n");
		if (usb_endpoint_is_isoc_in(endpoint))
			printk("ISOC IN\n");

		endpoint = &interface->endpoint[1].desc;
		if (usb_endpoint_is_int_out(endpoint))
			printk("INT OUT\n");
		if (usb_endpoint_is_bulk_out(endpoint))
			printk("BULK OUT\n");
		if (usb_endpoint_is_isoc_out(endpoint))
			printk("ISOC OUT\n");



		/*if (!usb_endpoint_is_int_in(endpoint))
			return -ENODEV;*/

		for(i=0; i<interface->desc.bNumEndpoints; i++){
			endpoint = &interface->endpoint[i].desc;

			/*if (!andromon_usb->bulk_out_endpointAddr &&
				((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT) && 
				(endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK
			   ){
					andromon_usb->bulk_out_endpointAddr = endpoint->bEndpointAddress;
					Debug_Print("ANDROMON", "Bulk out endpoint");
				}*/

			if (((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN) && 
				(endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK
			   ){
					//andromon_usb->bulk_in_endpointAddr = endpoint->bEndpointAddress;
					printk("BEAGLEDROID: Bulk in endpoint\n");
					break;
				}
		}



		pipe = usb_rcvbulkpipe(dev, endpoint->bEndpointAddress);
		maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));

		kbd = kzalloc(sizeof(struct usb_kbd), GFP_KERNEL);
		input_dev = input_allocate_device();
		if (!kbd || !input_dev)
			goto fail1;

		if (usb_kbd_alloc_mem(dev, kbd))
			goto fail2;

		kbd->usbdev = dev;
		kbd->dev = input_dev;

		spin_lock_init(&kbd->leds_lock);

		if (dev->manufacturer){
			strlcpy(kbd->name, dev->manufacturer, sizeof(kbd->name));
			printk("Manufacturer: %s\n", kbd->name);
		}

		if (dev->product) {
			if (dev->manufacturer)
				strlcat(kbd->name, " ", sizeof(kbd->name));
			strlcat(kbd->name, dev->product, sizeof(kbd->name));
		}

		if (!strlen(kbd->name)){
			snprintf(kbd->name, sizeof(kbd->name),
				 "USB HIDBP Keyboard %04x:%04x",
				 le16_to_cpu(dev->descriptor.idVendor),
				 le16_to_cpu(dev->descriptor.idProduct));

			printk("Name = %s\n", kbd->name);
		}


		usb_make_path(dev, kbd->phys, sizeof(kbd->phys));
		strlcat(kbd->phys, "/input0", sizeof(kbd->phys));

		input_dev->name = kbd->name;
		input_dev->phys = kbd->phys;
		usb_to_input_id(dev, &input_dev->id);
		input_dev->dev.parent = &iface->dev;

		input_set_drvdata(input_dev, kbd);

		input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_LED) |
			BIT_MASK(EV_REP) | BIT_MASK(EV_REL);
		input_dev->ledbit[0] = BIT_MASK(LED_NUML) | BIT_MASK(LED_CAPSL) |
			BIT_MASK(LED_SCROLLL) | BIT_MASK(LED_COMPOSE) |
			BIT_MASK(LED_KANA);
		input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
		input_dev->keybit[BIT_WORD(BTN_MOUSE)] |= BIT_MASK(BTN_SIDE) |
				BIT_MASK(BTN_EXTRA);
		input_dev->relbit[0] |= BIT_MASK(REL_WHEEL);

		for (i = 0; i < 255; i++)
			set_bit(usb_kbd_keycode[i], input_dev->keybit);
		clear_bit(0, input_dev->keybit);

		input_dev->event = usb_kbd_event;
		input_dev->open = usb_kbd_open;
		input_dev->close = usb_kbd_close;


		usb_fill_int_urb(kbd->irq, dev, pipe,
				 kbd->new, (maxp > 8 ? 8 : maxp),
				 usb_kbd_irq, kbd, endpoint->bInterval);

		kbd->irq->transfer_dma = kbd->new_dma;
		kbd->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

		kbd->cr->bRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE;
		kbd->cr->bRequest = 0x09;
		kbd->cr->wValue = cpu_to_le16(0x200);
		kbd->cr->wIndex = cpu_to_le16(interface->desc.bInterfaceNumber);
		kbd->cr->wLength = cpu_to_le16(1);

		/*usb_fill_control_urb(kbd->led, dev, usb_sndctrlpipe(dev, 0),
					 (void *) kbd->cr, kbd->leds, 1,
					 usb_kbd_led, kbd);
		kbd->led->transfer_dma = kbd->leds_dma;
		kbd->led->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;*/

		error = input_register_device(kbd->dev);
		if (error)
			goto fail2;

		usb_set_intfdata(iface, kbd);
		device_set_wakeup_enable(&dev->dev, 1);
	
		return 0;
	}
	else{
		datalen = GetProtocol(dev, (char*)data);

		if (datalen < 0) {
			printk("BEAGLEDROID-KBD: usb_control_msg : [%d]", datalen);
		}
		else{
			printk("BEAGLEDROID-KBD: AOA version = %d\n", data[0]);
		}

		SendIdentificationInfo(dev, ID_MANU, (char*)MANU);
		SendIdentificationInfo(dev, ID_MODEL, (char*)MODEL);
		SendIdentificationInfo(dev, ID_DESC, (char*)DESCRIPTION);
		SendIdentificationInfo(dev, ID_VERSION, (char*)VERSION);
		SendIdentificationInfo(dev, ID_URI, (char*)URI);
		SendIdentificationInfo(dev, ID_SERIAL, (char*)SERIAL);

		printk("<-----Audio Request------  %d ----------------->\n", SendAudioActivationRequest(dev));
		SendAOAActivationRequest(dev);
		//printk("<-----HID Request------  %d ----------------->\n", RegisterHID(dev));


		//input_report_key(dev, BTN_LEFT,   0x01);
		//input_report_key(dev, BTN_RIGHT,  0x01);
		//input_report_key(dev, BTN_MIDDLE, 0x04);
		//input_report_key(dev, BTN_SIDE,   0x08);
		//input_report_key(dev, BTN_EXTRA,  0x10);

		return 0;
	}

fail2:	
	usb_kbd_free_mem(dev, kbd);
fail1:	
	input_free_device(input_dev);
	kfree(kbd);
	return error;
}

static void usb_kbd_disconnect(struct usb_interface *intf)
{
	struct usb_kbd *kbd = usb_get_intfdata (intf);

	usb_set_intfdata(intf, NULL);
	if (kbd) {
		usb_kill_urb(kbd->irq);
		input_unregister_device(kbd->dev);
		//usb_kill_urb(kbd->led);
		usb_kbd_free_mem(interface_to_usbdev(intf), kbd);
		kfree(kbd);
	}
}

// information is obtained using "lsusb" at the command line
static struct usb_device_id usb_kbd_id_table [] = {
	/*{ USB_DEVICE(0x18d1, 0x4e42) },
	{ USB_DEVICE_INTERFACE_NUMBER(0x18d1, 0x2d05, 0) },
	{ USB_DEVICE_INTERFACE_NUMBER(0x18d1, 0x2d05, 1) },
	{ USB_DEVICE_INTERFACE_NUMBER(0x18d1, 0x2d05, 2) },
	{ USB_DEVICE_INTERFACE_NUMBER(0x18d1, 0x2d05, 3) },*/


	{ USB_DEVICE(0x18d1, 0x4e41) },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x18d1, 0x4e42, 255, 255, 0) },	
	{ USB_DEVICE_AND_INTERFACE_INFO(0x18d1, 0x2d01, 255, 255, 0) },	
	{ USB_DEVICE_AND_INTERFACE_INFO(0x18d1, 0x2d04, 255, 255, 0) },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x18d1, 0x2d05, 255, 255, 0) },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x18d1, 0x2d05, 1, 1, 0) },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x18d1, 0x2d05, 1, 2, 0) },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x18d1, 0x2d05, 255, 66, 1) },
	{ USB_DEVICE_AND_INTERFACE_INFO(0x05c6, 0x6764, 255, 255, 0) },	
	{ USB_DEVICE_AND_INTERFACE_INFO(0x05c6, 0x6765, 255, 255, 0) },	
	/*{ USB_DEVICE(0x1005, 0xb113) },
	{ USB_DEVICE(0x05c6, 0x6765) },	// OnePlus One
	{ USB_DEVICE(0x04e8, 0x6860) },	// Samsung S4
	{ USB_DEVICE(0x18d1, 0x4e41) },	// Nexus 7
	{ USB_DEVICE(0x18d1, 0x4e42) },	// Nexus 7
	{ USB_DEVICE(0x18d1, 0x2d00) },	// Nexus 7 AOA
	{ USB_DEVICE(0x18d1, 0x2d01) },	// Nexus 7 AOA
	{ USB_DEVICE(0x18d1, 0x2d02) },	// Nexus 7 AOA
	{ USB_DEVICE(0x18d1, 0x2d03) },	// Nexus 7 AOA
	{ USB_DEVICE(0x18d1, 0x2d04) },	// Nexus 7 AOA
	{ USB_DEVICE(0x18d1, 0x2d05) },	// Nexus 7 AOA
	{ USB_DEVICE(0x0bb4, 0x0cb0) },	// HTC wildfire*/
	{}	/* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, usb_kbd_id_table);

static struct usb_driver usb_kbd_driver = {
	.name =		"usbkbd",
	.probe =	usb_kbd_probe,
	.disconnect =	usb_kbd_disconnect,
	.id_table =	usb_kbd_id_table,
};

module_usb_driver(usb_kbd_driver);
