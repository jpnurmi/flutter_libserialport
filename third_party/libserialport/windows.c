/*
 * This file is part of the libserialport project.
 *
 * Copyright (C) 2013-2014 Martin Ling <martin-libserialport@earth.li>
 * Copyright (C) 2014 Aurelien Jacobs <aurel@gnuage.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "libserialport_internal.h"

/* USB path is a string of at most 8 decimal numbers < 128 separated by dots. */
#define MAX_USB_PATH ((8 * 3) + (7 * 1) + 1)

static void enumerate_hub(struct sp_port *port, const char *hub_name,
                          const char *parent_path, DEVINST dev_inst);

static char *wc_to_utf8(PWCHAR wc_buffer, ULONG wc_bytes)
{
	ULONG wc_length = wc_bytes / sizeof(WCHAR);
	ULONG utf8_bytes;
	WCHAR *wc_str = NULL;
	char *utf8_str = NULL;

	/* Allocate aligned wide char buffer */
	if (!(wc_str = malloc((wc_length + 1) * sizeof(WCHAR))))
		goto wc_to_utf8_end;

	/* Zero-terminate the wide char string. */
	memcpy(wc_str, wc_buffer, wc_bytes);
	wc_str[wc_length] = 0;

	/* Compute the size of the UTF-8 converted string. */
	if (!(utf8_bytes = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wc_str, -1,
	                                 NULL, 0, NULL, NULL)))
		goto wc_to_utf8_end;

	/* Allocate UTF-8 output buffer. */
	if (!(utf8_str = malloc(utf8_bytes)))
		goto wc_to_utf8_end;

	/* Actually converted to UTF-8. */
	if (!WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, wc_str, -1,
	                         utf8_str, utf8_bytes, NULL, NULL)) {
		free(utf8_str);
		utf8_str = NULL;
		goto wc_to_utf8_end;
	}

wc_to_utf8_end:
	if (wc_str)
		free(wc_str);

	return utf8_str;
}

static char *get_root_hub_name(HANDLE host_controller)
{
	USB_ROOT_HUB_NAME root_hub_name;
	PUSB_ROOT_HUB_NAME root_hub_name_wc;
	char *root_hub_name_utf8;
	ULONG size = 0;

	/* Compute the size of the root hub name string. */
	if (!DeviceIoControl(host_controller, IOCTL_USB_GET_ROOT_HUB_NAME, 0, 0,
	                     &root_hub_name, sizeof(root_hub_name), &size, NULL))
		return NULL;

	/* Allocate wide char root hub name string. */
	size = root_hub_name.ActualLength;
	if (!(root_hub_name_wc = malloc(size)))
		return NULL;

	/* Actually get the root hub name string. */
	if (!DeviceIoControl(host_controller, IOCTL_USB_GET_ROOT_HUB_NAME,
	                     NULL, 0, root_hub_name_wc, size, &size, NULL)) {
		free(root_hub_name_wc);
		return NULL;
	}

	/* Convert the root hub name string to UTF-8. */
	root_hub_name_utf8 = wc_to_utf8(root_hub_name_wc->RootHubName, size - offsetof(USB_ROOT_HUB_NAME, RootHubName));
	free(root_hub_name_wc);
	return root_hub_name_utf8;
}

static char *get_external_hub_name(HANDLE hub, ULONG connection_index)
{
	USB_NODE_CONNECTION_NAME ext_hub_name;
	PUSB_NODE_CONNECTION_NAME ext_hub_name_wc;
	char *ext_hub_name_utf8;
	ULONG size;

	/* Compute the size of the external hub name string. */
	ext_hub_name.ConnectionIndex = connection_index;
	if (!DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_NAME,
	                     &ext_hub_name, sizeof(ext_hub_name),
	                     &ext_hub_name, sizeof(ext_hub_name), &size, NULL))
		return NULL;

	/* Allocate wide char external hub name string. */
	size = ext_hub_name.ActualLength;
	if (size <= sizeof(ext_hub_name)
	    || !(ext_hub_name_wc = malloc(size)))
		return NULL;

	/* Get the name of the external hub attached to the specified port. */
	ext_hub_name_wc->ConnectionIndex = connection_index;
	if (!DeviceIoControl(hub, IOCTL_USB_GET_NODE_CONNECTION_NAME,
	                     ext_hub_name_wc, size,
	                     ext_hub_name_wc, size, &size, NULL)) {
		free(ext_hub_name_wc);
		return NULL;
	}

	/* Convert the external hub name string to UTF-8. */
	ext_hub_name_utf8 = wc_to_utf8(ext_hub_name_wc->NodeName, size - offsetof(USB_NODE_CONNECTION_NAME, NodeName));
	free(ext_hub_name_wc);
	return ext_hub_name_utf8;
}

static char *get_string_descriptor(HANDLE hub_device, ULONG connection_index,
                                   UCHAR descriptor_index)
{
	char desc_req_buf[sizeof(USB_DESCRIPTOR_REQUEST) +
	                  MAXIMUM_USB_STRING_LENGTH] = { 0 };
	PUSB_DESCRIPTOR_REQUEST desc_req = (void *)desc_req_buf;
	PUSB_STRING_DESCRIPTOR desc = (void *)(desc_req + 1);
	ULONG size = sizeof(desc_req_buf);

	desc_req->ConnectionIndex = connection_index;
	desc_req->SetupPacket.wValue = (USB_STRING_DESCRIPTOR_TYPE << 8)
	                               | descriptor_index;
	desc_req->SetupPacket.wIndex = 0;
	desc_req->SetupPacket.wLength = (USHORT) (size - sizeof(*desc_req));

	if (!DeviceIoControl(hub_device,
	                     IOCTL_USB_GET_DESCRIPTOR_FROM_NODE_CONNECTION,
	                     desc_req, size, desc_req, size, &size, NULL)
	    || size < 2
	    || desc->bDescriptorType != USB_STRING_DESCRIPTOR_TYPE
	    || desc->bLength != size - sizeof(*desc_req)
	    || desc->bLength % 2)
		return NULL;

	return wc_to_utf8(desc->bString, desc->bLength - offsetof(USB_STRING_DESCRIPTOR, bString));
}

static void enumerate_hub_ports(struct sp_port *port, HANDLE hub_device,
                                ULONG nb_ports, const char *parent_path, DEVINST dev_inst)
{
	char path[MAX_USB_PATH];
	ULONG index = 0;

	for (index = 1; index <= nb_ports; index++) {
		PUSB_NODE_CONNECTION_INFORMATION_EX connection_info_ex;
		ULONG size = sizeof(*connection_info_ex) + (30 * sizeof(USB_PIPE_INFO));

		if (!(connection_info_ex = malloc(size)))
			break;

		connection_info_ex->ConnectionIndex = index;
		if (!DeviceIoControl(hub_device,
		                     IOCTL_USB_GET_NODE_CONNECTION_INFORMATION_EX,
		                     connection_info_ex, size,
		                     connection_info_ex, size, &size, NULL)) {
			/*
			 * Try to get CONNECTION_INFORMATION if
			 * CONNECTION_INFORMATION_EX did not work.
			 */
			PUSB_NODE_CONNECTION_INFORMATION connection_info;

			size = sizeof(*connection_info) + (30 * sizeof(USB_PIPE_INFO));
			if (!(connection_info = malloc(size))) {
				free(connection_info_ex);
				continue;
			}
			connection_info->ConnectionIndex = index;
			if (!DeviceIoControl(hub_device,
			                     IOCTL_USB_GET_NODE_CONNECTION_INFORMATION,
			                     connection_info, size,
			                     connection_info, size, &size, NULL)) {
				free(connection_info);
				free(connection_info_ex);
				continue;
			}

			connection_info_ex->ConnectionIndex = connection_info->ConnectionIndex;
			connection_info_ex->DeviceDescriptor = connection_info->DeviceDescriptor;
			connection_info_ex->DeviceIsHub = connection_info->DeviceIsHub;
			connection_info_ex->DeviceAddress = connection_info->DeviceAddress;
			free(connection_info);
		}

		if (connection_info_ex->DeviceIsHub) {
			/* Recursively enumerate external hub. */
			PCHAR ext_hub_name;
			if ((ext_hub_name = get_external_hub_name(hub_device, index))) {
				snprintf(path, sizeof(path), "%s%ld.",
				         parent_path, connection_info_ex->ConnectionIndex);
				enumerate_hub(port, ext_hub_name, path, dev_inst);
			}
			free(connection_info_ex);
		} else {
			snprintf(path, sizeof(path), "%s%ld",
			         parent_path, connection_info_ex->ConnectionIndex);

			/* Check if this device is the one we search for. */
			if (strcmp(path, port->usb_path)) {
				free(connection_info_ex);
				continue;
			}

			/* Finally grab detailed information regarding the device. */
			port->usb_address = connection_info_ex->DeviceAddress + 1;
			/*port->usb_vid = connection_info_ex->DeviceDescriptor.idVendor;
			port->usb_pid = connection_info_ex->DeviceDescriptor.idProduct;*/

			if (connection_info_ex->DeviceDescriptor.iManufacturer)
				port->usb_manufacturer = get_string_descriptor(hub_device, index,
				           connection_info_ex->DeviceDescriptor.iManufacturer);
			if (connection_info_ex->DeviceDescriptor.iProduct)
				port->usb_product = get_string_descriptor(hub_device, index,
				           connection_info_ex->DeviceDescriptor.iProduct);
			if (connection_info_ex->DeviceDescriptor.iSerialNumber) {
				port->usb_serial = get_string_descriptor(hub_device, index,
				           connection_info_ex->DeviceDescriptor.iSerialNumber);
				if (port->usb_serial == NULL) {
					//composite device, get the parent's serial number
					char device_id[MAX_DEVICE_ID_LEN];
					if (CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS) {
						if (CM_Get_Device_IDA(dev_inst, device_id, sizeof(device_id), 0) == CR_SUCCESS)
							port->usb_serial = strdup(strrchr(device_id, '\\')+1);
					}
				}
			}

			free(connection_info_ex);
			break;
		}
	}
}

static void enumerate_hub(struct sp_port *port, const char *hub_name,
                          const char *parent_path, DEVINST dev_inst)
{
	USB_NODE_INFORMATION hub_info;
	HANDLE hub_device;
	ULONG size = sizeof(hub_info);
	char *device_name;

	/* Open the hub with its full name. */
	if (!(device_name = malloc(strlen("\\\\.\\") + strlen(hub_name) + 1)))
		return;
	strcpy(device_name, "\\\\.\\");
	strcat(device_name, hub_name);
	hub_device = CreateFileA(device_name, GENERIC_WRITE, FILE_SHARE_WRITE,
	                         NULL, OPEN_EXISTING, 0, NULL);
	free(device_name);
	if (hub_device == INVALID_HANDLE_VALUE)
		return;

	/* Get the number of ports of the hub. */
	if (DeviceIoControl(hub_device, IOCTL_USB_GET_NODE_INFORMATION,
	                    &hub_info, size, &hub_info, size, &size, NULL))
		/* Enumerate the ports of the hub. */
		enumerate_hub_ports(port, hub_device,
		   hub_info.u.HubInformation.HubDescriptor.bNumberOfPorts, parent_path, dev_inst);

	CloseHandle(hub_device);
}

static void enumerate_host_controller(struct sp_port *port,
                                      HANDLE host_controller_device,
                                      DEVINST dev_inst)
{
	char *root_hub_name;

	if ((root_hub_name = get_root_hub_name(host_controller_device))) {
		enumerate_hub(port, root_hub_name, "", dev_inst);
		free(root_hub_name);
	}
}

static void get_usb_details(struct sp_port *port, DEVINST dev_inst_match)
{
	HDEVINFO device_info;
	SP_DEVINFO_DATA device_info_data;
	ULONG i, size = 0;

	device_info = SetupDiGetClassDevs(&GUID_CLASS_USB_HOST_CONTROLLER, NULL, NULL,
	                                  DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	device_info_data.cbSize = sizeof(device_info_data);

	for (i = 0; SetupDiEnumDeviceInfo(device_info, i, &device_info_data); i++) {
		SP_DEVICE_INTERFACE_DATA device_interface_data;
		PSP_DEVICE_INTERFACE_DETAIL_DATA device_detail_data;
		DEVINST dev_inst = dev_inst_match;
		HANDLE host_controller_device;

		device_interface_data.cbSize = sizeof(device_interface_data);
		if (!SetupDiEnumDeviceInterfaces(device_info, 0,
		                                 &GUID_CLASS_USB_HOST_CONTROLLER,
		                                 i, &device_interface_data))
			continue;

		if (!SetupDiGetDeviceInterfaceDetail(device_info,&device_interface_data,
		                                     NULL, 0, &size, NULL)
		    && GetLastError() != ERROR_INSUFFICIENT_BUFFER)
			continue;

		if (!(device_detail_data = malloc(size)))
			continue;
		device_detail_data->cbSize = sizeof(*device_detail_data);
		if (!SetupDiGetDeviceInterfaceDetail(device_info,&device_interface_data,
		                                     device_detail_data, size, &size,
		                                     NULL)) {
			free(device_detail_data);
			continue;
		}

		while (CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS
		       && dev_inst != device_info_data.DevInst) { }
		if (dev_inst != device_info_data.DevInst) {
			free(device_detail_data);
			continue;
		}

		port->usb_bus = i + 1;

		host_controller_device = CreateFile(device_detail_data->DevicePath,
		                                    GENERIC_WRITE, FILE_SHARE_WRITE,
		                                    NULL, OPEN_EXISTING, 0, NULL);
		if (host_controller_device != INVALID_HANDLE_VALUE) {
			enumerate_host_controller(port, host_controller_device, dev_inst_match);
			CloseHandle(host_controller_device);
		}
		free(device_detail_data);
	}

	SetupDiDestroyDeviceInfoList(device_info);
	return;
}

SP_PRIV enum sp_return get_port_details(struct sp_port *port)
{
	/*
	 * Description limited to 127 char, anything longer
	 * would not be user friendly anyway.
	 */
	char description[128];
	SP_DEVINFO_DATA device_info_data = { .cbSize = sizeof(device_info_data) };
	HDEVINFO device_info;
	int i;

	device_info = SetupDiGetClassDevs(NULL, 0, 0,
	                                  DIGCF_PRESENT | DIGCF_ALLCLASSES);
	if (device_info == INVALID_HANDLE_VALUE)
		RETURN_FAIL("SetupDiGetClassDevs() failed");

	for (i = 0; SetupDiEnumDeviceInfo(device_info, i, &device_info_data); i++) {
		HKEY device_key;
		DEVINST dev_inst;
		char value[8], class[16];
		DWORD size, type;
		CONFIGRET cr;

		/* Check if this is the device we are looking for. */
		device_key = SetupDiOpenDevRegKey(device_info, &device_info_data,
		                                  DICS_FLAG_GLOBAL, 0,
		                                  DIREG_DEV, KEY_QUERY_VALUE);
		if (device_key == INVALID_HANDLE_VALUE)
			continue;
		size = sizeof(value);
		if (RegQueryValueExA(device_key, "PortName", NULL, &type, (LPBYTE)value,
		                     &size) != ERROR_SUCCESS || type != REG_SZ) {
			RegCloseKey(device_key);
			continue;
		}
		RegCloseKey(device_key);
		value[sizeof(value) - 1] = 0;
		if (strcmp(value, port->name))
			continue;

		/* Check port transport type. */
		dev_inst = device_info_data.DevInst;
		size = sizeof(class);
		cr = CR_FAILURE;
		while (CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS &&
		       (cr = CM_Get_DevNode_Registry_PropertyA(dev_inst,
		                 CM_DRP_CLASS, 0, class, &size, 0)) != CR_SUCCESS) { }
		if (cr == CR_SUCCESS) {
			if (!strcmp(class, "USB"))
				port->transport = SP_TRANSPORT_USB;
		}

		/* Get port description (friendly name). */
		dev_inst = device_info_data.DevInst;
		size = sizeof(description);
		while ((cr = CM_Get_DevNode_Registry_PropertyA(dev_inst,
		          CM_DRP_FRIENDLYNAME, 0, description, &size, 0)) != CR_SUCCESS
		       && CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS) { }
		if (cr == CR_SUCCESS)
			port->description = strdup(description);

		/* Get more informations for USB connected ports. */
		if (port->transport == SP_TRANSPORT_USB) {
			char usb_path[MAX_USB_PATH] = "", tmp[MAX_USB_PATH];
			char hardware_ids[512];
			char device_id[MAX_DEVICE_ID_LEN];

			/* Recurse over parents to build the USB device path. */
			dev_inst = device_info_data.DevInst;
			do {
				/* Verify that this layer of the tree is USB related. */
				if (CM_Get_Device_IDA(dev_inst, device_id,
				                      sizeof(device_id), 0) != CR_SUCCESS
				    || strncmp(device_id, "USB\\", 4))
					continue;

				/* Discard one layer for composite devices. */
				char compat_ids[512], *p = compat_ids;
				size = sizeof(compat_ids);
				if (CM_Get_DevNode_Registry_PropertyA(dev_inst,
				                                      CM_DRP_COMPATIBLEIDS, 0,
				                                      &compat_ids,
				                                      &size, 0) == CR_SUCCESS) {
					while (*p) {
						if (!strncmp(p, "USB\\COMPOSITE", 13))
							break;
						p += strlen(p) + 1;
					}
					if (*p)
						continue;
				}

				/* Stop the recursion when reaching the USB root. */
				if (!strncmp(device_id, "USB\\ROOT", 8))
					break;

				/* Prepend the address of current USB layer to the USB path. */
				DWORD address;
				size = sizeof(address);
				if (CM_Get_DevNode_Registry_PropertyA(dev_inst, CM_DRP_ADDRESS,
				                        0, &address, &size, 0) == CR_SUCCESS) {
					strcpy(tmp, usb_path);
					snprintf(usb_path, sizeof(usb_path), "%d%s%s",
					         (int)address, *tmp ? "." : "", tmp);
				}

				/* Grab hardware ids. */
				size = sizeof(hardware_ids);
				if (CM_Get_DevNode_Registry_PropertyA(dev_inst, CM_DRP_HARDWAREID                  ,
				                        0, &hardware_ids, &size, 0) == CR_SUCCESS) {
					//printf("Result hardware ids: %s\n", hardware_ids);
				}
			} while (CM_Get_Parent(&dev_inst, dev_inst, 0) == CR_SUCCESS);

			port->usb_path = strdup(usb_path);
			
			char *result = strstr(hardware_ids, "MI_");
			size_t MI_idx = result - hardware_ids;
			if((MI_idx >= 0) && ((MI_idx + 4) < sizeof(hardware_ids))) {
				int a = hardware_ids[MI_idx + 3] - '0';
				int b = hardware_ids[MI_idx + 4] -'0';
				port->usb_interface_number = a * 10 + b;
			}

			
			result = strstr(hardware_ids, "VID_");
			size_t VID_idx = result - hardware_ids;
			if((VID_idx >= 0) && ((VID_idx + 7) < sizeof(hardware_ids))) {
				sscanf((hardware_ids + VID_idx + 4), "%4x", &port->usb_vid);
			}

			result = strstr(hardware_ids, "PID_");
			size_t PID_idx = result - hardware_ids;
			if((PID_idx >= 0) && ((PID_idx + 7) < sizeof(hardware_ids))) {
				sscanf((hardware_ids + PID_idx + 4), "%4x", &port->usb_pid);
			}

			/* Wake up the USB device to be able to read string descriptor. */
			char *escaped_port_name;
			HANDLE handle;
			if (!(escaped_port_name = malloc(strlen(port->name) + 5)))
				RETURN_ERROR(SP_ERR_MEM, "Escaped port name malloc failed");
			sprintf(escaped_port_name, "\\\\.\\%s", port->name);
			handle = CreateFileA(escaped_port_name, GENERIC_READ, 0, 0,
			                     OPEN_EXISTING,
			                     FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED, 0);
			free(escaped_port_name);
			CloseHandle(handle);

			/* Retrieve USB device details from the device descriptor. */
			get_usb_details(port, device_info_data.DevInst);
		}
		break;
	}

	SetupDiDestroyDeviceInfoList(device_info);

	RETURN_OK();
}

SP_PRIV enum sp_return list_ports(struct sp_port ***list)
{
	HKEY key;
	TCHAR *value, *data;
	DWORD max_value_len, max_data_size, max_data_len;
	DWORD value_len, data_size, data_len;
	DWORD type, index = 0;
	LSTATUS result;
	char *name;
	int name_len;
	int ret = SP_OK;

	DEBUG("Opening registry key");
	if ((result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"),
			0, KEY_QUERY_VALUE, &key)) != ERROR_SUCCESS) {
		/* It's possible for this key to not exist if there are no serial ports
		 * at all. In that case we're done. Return a failure for any other error. */
		if (result != ERROR_FILE_NOT_FOUND) {
			SetLastError(result);
			SET_FAIL(ret, "RegOpenKeyEx() failed");
		}
		goto out_done;
	}
	DEBUG("Querying registry key value and data sizes");
	if ((result = RegQueryInfoKey(key, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
				&max_value_len, &max_data_size, NULL, NULL)) != ERROR_SUCCESS) {
		SetLastError(result);
		SET_FAIL(ret, "RegQueryInfoKey() failed");
		goto out_close;
	}
	max_data_len = max_data_size / sizeof(TCHAR);
	if (!(value = malloc((max_value_len + 1) * sizeof(TCHAR)))) {
		SET_ERROR(ret, SP_ERR_MEM, "Registry value malloc failed");
		goto out_close;
	}
	if (!(data = malloc((max_data_len + 1) * sizeof(TCHAR)))) {
		SET_ERROR(ret, SP_ERR_MEM, "Registry data malloc failed");
		goto out_free_value;
	}
	DEBUG("Iterating over values");
	while (
		value_len = max_value_len + 1,
		data_size = max_data_size,
		RegEnumValue(key, index, value, &value_len,
			NULL, &type, (LPBYTE)data, &data_size) == ERROR_SUCCESS)
	{
		if (type == REG_SZ) {
			data_len = data_size / sizeof(TCHAR);
			data[data_len] = '\0';
#ifdef UNICODE
			name_len = WideCharToMultiByte(CP_ACP, 0, data, -1, NULL, 0, NULL, NULL);
#else
			name_len = data_len + 1;
#endif
			if (!(name = malloc(name_len))) {
				SET_ERROR(ret, SP_ERR_MEM, "Registry port name malloc failed");
				goto out;
			}
#ifdef UNICODE
			WideCharToMultiByte(CP_ACP, 0, data, -1, name, name_len, NULL, NULL);
#else
			strcpy(name, data);
#endif
			DEBUG_FMT("Found port %s", name);
			if (!(*list = list_append(*list, name))) {
				SET_ERROR(ret, SP_ERR_MEM, "List append failed");
				free(name);
				goto out;
			}
			free(name);
		}
		index++;
	}
out:
	free(data);
out_free_value:
	free(value);
out_close:
	RegCloseKey(key);
out_done:

	return ret;
}
