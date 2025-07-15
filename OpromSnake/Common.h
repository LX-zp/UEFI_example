#ifndef _COMMON_H
#define _COMMON_H

//所有包含的目录到mdepkg下去找
#include  <Uefi.h>
#include  <Library/UefiLib.h>
// #include  <Library/ShellCEntryLib.h>
#include  <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseLib.h>
#include <Library/PrintLib.h>
//#include <Protocol/SimpleTextInEx.h>
//#include <Library/PciLib.h>
//#include <Library/PciExpressLib.h>
//#include <IndustryStandard/Pci.h>
#include <Protocol/PciIo.h>						//获取PciIO protocol所需
#include <Protocol/PciRootBridgeIo.h>	//获取PciRootBridgeIO protocol所需
#include <IndustryStandard/Pci.h>  //pci访问所需的头文件，包含pci22.h,pci23.h...
#include <Protocol/Rng.h>  //Random Number Generator Protocol 2019-08-31 11:32:03 robin
#include <Protocol/SimpleFileSystem.h> //文件系统访问
#include <IndustryStandard/Bmp.h> //for bmp
#include <Library/MemoryAllocationLib.h> //AllocatePool function...
#include <guid/FileInfo.h> //FileInfo
#include <Library/FileHandleLib.h>  //文件信息处理
#include <Protocol/HiiFont.h>
#include <Protocol/HiiImage.h>
#include <Protocol/HiiImageEx.h>
#include <Protocol/HiiImageDecoder.h>
#include <Protocol/HiiDatabase.h>
#include <Library/HiiLib.h>
#include <Library/UefiHiiServicesLib.h>  //gHiiString

#include "Snake.h"
#include "Keyboard.h"
#include "Graphic.h"
#include "Font.h"
#include "Window.h"

#define S_TEXT_INPUT_EX  		0x0000000000000001
#define GRAPHICS_OUTPUT  		0x0000000000000002
#define PCI_ROOTBRIDGE_IO		0x0000000000000004
#define PCI_IO							0x0000000000000008
#define FILE_IO             0x0000000000000010  //2019-06-10 19:53:27 luobing
#define HII_FONT						0x0000000000000020  //2019-6-8 7:39:47 luobing
#define HII_IMAGE           0x0000000000000040  //2020-9-27 16:32:22 luobing
#define HII_IMAGE_EX        0x0000000000000080  //2020-9-27 16:32:23 luobing
#define HII_IMAGE_DECODER   0x0000000000000100  //2020-9-27 16:32:24 luobing
#define RNG_OUT             0x0000000000000200  //2019-08-31 11:33:18 robin

UINT64 InintGloabalProtocols(UINT64 flag);
EFI_STATUS LocateSimpleTextInputEx(void);
EFI_STATUS LocateGraphicsOutput (void);
EFI_STATUS LocatePCIRootBridgeIO(void);
EFI_STATUS LocatePCIIO(void);
EFI_STATUS LocateFileRoot(void);

EFI_STATUS LocateHiiFont(void);  //2019-6-8 7:38:56 luobing
EFI_STATUS LocateHiiImage(void);
EFI_STATUS LocateHiiImageEx(void);
EFI_STATUS LocateHiiImageDecoder(void);
EFI_STATUS LocateRNGOut(void);
VOID Delayms(IN UINTN ms);


#define BLANKDRV_DEV_SIGNATURE SIGNATURE_32 ('s', 'e', 'n', 'y')
#define CH366_VENDOR_ID 0x1C00
#define CH366_DEVICE_ID 0x4349

typedef struct {
  UINT32                    Signature;
  EFI_HANDLE                Handle;
  VOID			                *BlankDrvVariable;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL   SimpleTextIn;
  UINT8                     DeviceType;
  BOOLEAN                   FixedDevice;
  UINT16                    Reserved;
  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;

} BLANKDRV_DEV;

#define BLANKDRV_DEV_FROM_THIS(a)  CR (a, BLANKDRV_DEV, BlankDrvVariable, BLANKDRV_DEV_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL   gBlankDrvDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL   gBlankDrvComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL  gBlankDrvComponentName2;
extern EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL   *gPCIRootBridgeIO;
extern EFI_GRAPHICS_OUTPUT_PROTOCOL       *gGraphicsOutput;
//extern EFI_SIMPLE_POINTER_PROTOCOL        *gMouse;
extern EFI_GRAPHICS_OUTPUT_BLT_PIXEL       gColorTable[];
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
BlankDrvDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath   OPTIONAL
  );

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
  );

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
  );

//
// EFI Component Name Functions
//
/**
  Retrieves a Unicode string that is the user readable name of the driver.

  This function retrieves the user readable name of a driver in the form of a
  Unicode string. If the driver specified by This has a user readable name in
  the language specified by Language, then a pointer to the driver name is
  returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
  by This does not support the language specified by Language,
  then EFI_UNSUPPORTED is returned.

  @param  This                  A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  Language              A pointer to a Null-terminated ASCII string
                                array indicating the language. This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified
                                in RFC 4646 or ISO 639-2 language code format.

  @param  DriverName            A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                driver specified by This in the language
                                specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by
                                This and the language specified by Language was
                                returned in DriverName.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER DriverName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
BlankDrvComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  );


/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by a driver.

  This function retrieves the user readable name of the controller specified by
  ControllerHandle and ChildHandle in the form of a Unicode string. If the
  driver specified by This has a user readable name in the language specified by
  Language, then a pointer to the controller name is returned in ControllerName,
  and EFI_SUCCESS is returned.  If the driver specified by This is not currently
  managing the controller specified by ControllerHandle and ChildHandle,
  then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
  support the language specified by Language, then EFI_UNSUPPORTED is returned.

  @param  This                  A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
                                EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  ControllerHandle      The handle of a controller that the driver
                                specified by This is managing.  This handle
                                specifies the controller whose name is to be
                                returned.

  @param  ChildHandle           The handle of the child controller to retrieve
                                the name of.  This is an optional parameter that
                                may be NULL.  It will be NULL for device
                                drivers.  It will also be NULL for a bus drivers
                                that wish to retrieve the name of the bus
                                controller.  It will not be NULL for a bus
                                driver that wishes to retrieve the name of a
                                child controller.

  @param  Language              A pointer to a Null-terminated ASCII string
                                array indicating the language.  This is the
                                language of the driver name that the caller is
                                requesting, and it must match one of the
                                languages specified in SupportedLanguages. The
                                number of languages supported by a driver is up
                                to the driver writer. Language is specified in
                                RFC 4646 or ISO 639-2 language code format.

  @param  ControllerName        A pointer to the Unicode string to return.
                                This Unicode string is the name of the
                                controller specified by ControllerHandle and
                                ChildHandle in the language specified by
                                Language from the point of view of the driver
                                specified by This.

  @retval EFI_SUCCESS           The Unicode string for the user readable name in
                                the language specified by Language for the
                                driver specified by This was returned in
                                DriverName.

  @retval EFI_INVALID_PARAMETER ControllerHandle is not a valid EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid
                                EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER ControllerName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This is not currently
                                managing the controller specified by
                                ControllerHandle and ChildHandle.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
                                the language specified by Language.

**/
EFI_STATUS
EFIAPI
BlankDrvComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  );












#endif