#include "Common.h"

UINT16 MyIoBaseAddr = 0;

EFI_DRIVER_BINDING_PROTOCOL gBlankDrvDriverBinding = {
    BlankDrvDriverBindingSupported,
    BlankDrvDriverBindingStart,
    BlankDrvDriverBindingStop,
    0xa,
    NULL,
    NULL
};

/**
  The user Entry Point for module BlankDrv.
  The user code starts with this function.
  @param  ImageHandle    The firmware allocated handle for the EFI image.  
  @param  SystemTable    A pointer to the EFI System Table.
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.
**/
EFI_STATUS
EFIAPI
UefiMain(
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
)
{
    EFI_STATUS          Status;

    // Install driver model protocol(s).
    Status = EfiLibInstallDriverBindingComponentName2(
             ImageHandle,
             SystemTable,
             &gBlankDrvDriverBinding,
             ImageHandle,
             &gBlankDrvComponentName,
             &gBlankDrvComponentName2
             );
    ASSERT_EFI_ERROR (Status);

    return Status;
}


/**
  Test to see if this driver supports ControllerHandle.
  This service is called by the EFI boot service ConnectController(). In order
  to make drivers as small as possible, there are a few calling restrictions for
  this service. ConnectController() must follow these calling restrictions.
  If any other agent wishes to call Supported() it must also follow these
  calling restrictions.

  @param  This                Protocol instance pointer.
  @param  ControllerHandle    Handle of device to test
  @param  RemainingDevicePath Optional parameter use to pick a specific child
                              device to start.

  @retval EFI_SUCCESS         This driver supports this device
  @retval EFI_ALREADY_STARTED This driver is already running on this device
  @retval other               This driver does not support this device

**/
EFI_STATUS
EFIAPI
BlankDrvDriverBindingSupported(
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
)
{
    EFI_STATUS            Status;
    UINT16 MyVendorID = 0;
    UINT16 MyDeviceID = 0;
    EFI_PCI_IO_PROTOCOL  *PciIo;

    // open PCI protocol
    Status = gBS->OpenProtocol (
                    Controller,
                    &gEfiPciIoProtocolGuid,
                    (VOID **) &PciIo,
                    This->DriverBindingHandle,
                    Controller,
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                    );
    if (EFI_ERROR (Status)) {
        return Status;
    }
    
    // read deviceID and vendorID
    Status = PciIo->Pci.Read (
            PciIo,
            EfiPciIoWidthUint16,
            0,  // 
            1,
            &MyVendorID
            );
    if (EFI_ERROR (Status)) {
        goto Done;
    }

    Status = PciIo->Pci.Read (
            PciIo,
            EfiPciIoWidthUint16,
            2,  // 
            1,
            &MyDeviceID
            );
    if (EFI_ERROR (Status)){
            goto Done;
        }
    //
    // Evaluate Vendor ID and Device ID
    //
    Status = EFI_SUCCESS;
    if (MyVendorID != CH366_VENDOR_ID || MyDeviceID != CH366_DEVICE_ID) {
    Status = EFI_UNSUPPORTED;
    goto Done;
    }

    Done:
    //
    // Close the PCI I/O Protocol
    //

    gBS->CloseProtocol (
            Controller,
            &gEfiPciIoProtocolGuid,
            This->DriverBindingHandle,
            Controller
            );

    return Status;
}


/**
  Start this driver on ControllerHandle.

  This service is called by the EFI boot service ConnectController(). In order
  to make drivers as small as possible, there are a few calling restrictions for
  this service. ConnectController() must follow these calling restrictions. If
  any other agent wishes to call Start() it must also follow these calling
  restrictions.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @retval EFI_SUCCESS          This driver is added to ControllerHandle
  @retval EFI_ALREADY_STARTED  This driver is already running on ControllerHandle
  @retval other                This driver does not support this device

**/
EFI_STATUS
EFIAPI
BlankDrvDriverBindingStart (
    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
    IN EFI_HANDLE                   Controller,
    IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
)
{
    EFI_STATUS            Status;
    EFI_PCI_IO_PROTOCOL  *PciIo;
    UINT8 Status_Command = 0;

// Open the PCI I/O Protocol
	Status = gBS->OpenProtocol (
								Controller,
								&gEfiPciIoProtocolGuid,
								(VOID **)&PciIo,
								This->DriverBindingHandle,
								Controller,
								EFI_OPEN_PROTOCOL_BY_DRIVER
								);
    if (EFI_ERROR (Status)) {
        goto Done;
    }
                
    // Read Io Base address
    Status = PciIo->Pci.Read (
            PciIo,
            EfiPciIoWidthUint16,
            0x10,
            1,
            &MyIoBaseAddr
            );
        if (EFI_ERROR (Status)) {
            goto Done;
        }

    // read command
    Status = PciIo->Pci.Read (
        PciIo,
        EfiPciIoWidthUint8,
        0x4,
        1,
        &Status_Command
        );

    if (EFI_ERROR (Status)) {
        goto Done;
    }

    Status_Command|=0x07;
    Status = PciIo->Pci.Write (
        PciIo,
        EfiPciIoWidthUint8,
        0x4,
        1,
        &Status_Command
        );

    if (EFI_ERROR (Status)) {
    goto Done;
    }

    SnakeProcess();

    Status = EFI_SUCCESS;
    Done:
    //
        // gST->ConOut->OutputString(gST->ConOut,L"please input key(ESC to exit):\n\r");
    return Status;

}


/**
  Stop this driver on ControllerHandle.

  This service is called by the EFI boot service DisconnectController().
  In order to make drivers as small as possible, there are a few calling
  restrictions for this service. DisconnectController() must follow these
  calling restrictions. If any other agent wishes to call Stop() it must
  also follow these calling restrictions.
  
  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
                            children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
BlankDrvDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer   OPTIONAL
  )
{
// Close the PCI I/O Protocol
    gBS->CloseProtocol(
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
    );
    return EFI_SUCCESS;
    //return EFI_UNSUPPORTED;
}


/**
  This is the default unload handle for all the network drivers.

  Disconnect the driver specified by ImageHandle from all the devices in the handle database.
  Uninstall all the protocols installed in the driver entry point.
  
  @param[in]  ImageHandle       The drivers' driver image.

  @retval EFI_SUCCESS           The image is unloaded.
  @retval Others                Failed to unload the image.

**/
EFI_STATUS
EFIAPI
DefaultUnload (
  IN EFI_HANDLE             ImageHandle
  )
{
    EFI_STATUS                        Status;
    EFI_HANDLE                        *DeviceHandleBuffer;
    UINTN                             DeviceHandleCount;
    UINTN                             Index;
    EFI_DRIVER_BINDING_PROTOCOL       *DriverBinding;
    EFI_COMPONENT_NAME_PROTOCOL       *ComponentName;
    EFI_COMPONENT_NAME2_PROTOCOL      *ComponentName2;

    //
    // Get the list of all the handles in the handle database.
    // If there is an error getting the list, then the unload
    // operation fails.
    //
    Status = gBS->LocateHandleBuffer (
                    AllHandles,
                    NULL,
                    NULL,
                    &DeviceHandleCount,
                    &DeviceHandleBuffer
                    );

    if (EFI_ERROR (Status)) {
        return Status;
    }

    //
    // Disconnect the driver specified by ImageHandle from all
    // the devices in the handle database.
    //
    for (Index = 0; Index < DeviceHandleCount; Index++) {
        Status = gBS->DisconnectController (
                        DeviceHandleBuffer[Index],
                        ImageHandle,
                        NULL
                        );
    }

    //
    // Uninstall all the protocols installed in the driver entry point
    //
    for (Index = 0; Index < DeviceHandleCount; Index++) {
        Status = gBS->HandleProtocol (
                        DeviceHandleBuffer[Index],
                        &gEfiDriverBindingProtocolGuid,
                        (VOID **) &DriverBinding
                        );

        if (EFI_ERROR (Status)) {
        continue;
        }

        if (DriverBinding->ImageHandle != ImageHandle) {
        continue;
        }

        gBS->UninstallProtocolInterface (
            ImageHandle,
            &gEfiDriverBindingProtocolGuid,
            DriverBinding
            );
        Status = gBS->HandleProtocol (
                        DeviceHandleBuffer[Index],
                        &gEfiComponentNameProtocolGuid,
                        (VOID **) &ComponentName
                        );
        if (!EFI_ERROR (Status)) {
        gBS->UninstallProtocolInterface (
                ImageHandle,
                &gEfiComponentNameProtocolGuid,
                ComponentName
                );
        }

        Status = gBS->HandleProtocol (
                        DeviceHandleBuffer[Index],
                        &gEfiComponentName2ProtocolGuid,
                        (VOID **) &ComponentName2
                        );
        if (!EFI_ERROR (Status)) {
        gBS->UninstallProtocolInterface (
                ImageHandle,
                &gEfiComponentName2ProtocolGuid,
                ComponentName2
                );
        }
    }

    //
    // Free the buffer containing the list of handles from the handle database
    //
    if (DeviceHandleBuffer != NULL) {
        gBS->FreePool (DeviceHandleBuffer);
    }

    return EFI_SUCCESS;
}