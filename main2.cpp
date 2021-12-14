#include <iostream>
#include <string>
#include <windows.h>
#include <setupapi.h>
#include <comdef.h>
#include <assert.h>

using namespace std;

void displayError(const char* msg) {
	cout << msg << endl;
	system("PAUSE");
	exit(0);
};

template <class T>
inline void releaseMemory(T& x)
{
	assert(x != NULL);
	delete[] x;
	x = NULL;
}

HANDLE hidDeviceObject;
GUID classGuid;
HMODULE hHidLib;
DWORD memberIndex = 0;
DWORD deviceInterfaceDetailDataSize;
DWORD requiredSize;

HDEVINFO deviceInfoSet;
SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = NULL;
SP_DEVINFO_DATA deviceInfoData;

string getRegistryPropertyString(HDEVINFO deviceInfoSet, PSP_DEVINFO_DATA deviceInfoData, DWORD property)
{
	DWORD propertyBufferSize = 0;
	char* propertyBuffer = NULL;

	SetupDiGetDeviceRegistryProperty(deviceInfoSet, deviceInfoData, property, NULL, NULL, 0, &propertyBufferSize);
	propertyBuffer = new char[(propertyBufferSize * sizeof(char))];
	bool result = SetupDiGetDeviceRegistryProperty(deviceInfoSet, deviceInfoData, property,NULL,(PBYTE)propertyBuffer, propertyBufferSize,NULL);
	if (!result)
		releaseMemory(propertyBuffer);
    //wstring wstr(propertyBuffer);
    //string str(wstr.begin(), wstr.end());
	//_bstr_t b(propertyBuffer);
	return propertyBuffer;
}


int main()
{
	SetConsoleOutputCP(1250);
	hHidLib = LoadLibrary("C:\\Windows\\system32\\HID.DLL");
	if (!hHidLib)
	{
		displayError("Error load HID.DLL.");
	}

	void(__stdcall * HidD_GetHidGuid)(OUT LPGUID HidGuid);
	(FARPROC&)HidD_GetHidGuid = GetProcAddress(hHidLib, "HidD_GetHidGuid");

	if (!HidD_GetHidGuid) {
		FreeLibrary(hHidLib);
		displayError("Not found GUID");
	}
	HidD_GetHidGuid(&classGuid);

	deviceInfoSet = SetupDiGetClassDevs(&classGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (deviceInfoSet != INVALID_HANDLE_VALUE)
	{
		deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

		SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &classGuid,
			memberIndex, &deviceInterfaceData);

		while (GetLastError() != ERROR_NO_MORE_ITEMS)
		{
			memberIndex++;

			deviceInterfaceData.cbSize = sizeof(deviceInterfaceData);

			SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, NULL, 0, &deviceInterfaceDetailDataSize, NULL);

			deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA) new DWORD[deviceInterfaceDetailDataSize];

			deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
			deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

			if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData,
				deviceInterfaceDetailData, deviceInterfaceDetailDataSize,
				&requiredSize, &deviceInfoData)) {
				cout << "ClassDescr: " << getRegistryPropertyString(deviceInfoSet, &deviceInfoData, SPDRP_CLASS) << endl;
				cout << "ClassGUID: " << getRegistryPropertyString(deviceInfoSet, &deviceInfoData, SPDRP_CLASSGUID) << endl;
				cout << "CompatibileIDs: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_COMPATIBLEIDS) << endl;
				cout << "ConfigFlags: " << getRegistryPropertyString(deviceInfoSet, &deviceInfoData, SPDRP_CONFIGFLAGS) << endl;
				cout << "DeviceDescr: " << getRegistryPropertyString(deviceInfoSet, &deviceInfoData, SPDRP_DEVICEDESC) << endl;
				cout << "Driver: " << getRegistryPropertyString(deviceInfoSet, &deviceInfoData, SPDRP_DRIVER) << endl;
				cout << "HardwareID: " << getRegistryPropertyString(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID) << endl;
				cout << "Mfg: " << getRegistryPropertyString(deviceInfoSet, &deviceInfoData, SPDRP_MFG) << endl;
				cout << "EnumeratorName: " << getRegistryPropertyString(deviceInfoSet, &deviceInfoData, SPDRP_ENUMERATOR_NAME) << endl;
				cout << "PhysDevObjName: " << getRegistryPropertyString(deviceInfoSet,&deviceInfoData, SPDRP_PHYSICAL_DEVICE_OBJECT_NAME) << endl;
				cout << endl;
			}

			//releaseMemory(deviceInterfaceDetailData);

			SetupDiEnumDeviceInterfaces(deviceInfoSet, NULL, &classGuid,
				memberIndex, &deviceInterfaceData);
		}
		//SetupDiDestroyDeviceInfoList(deviceInfoSet);
		FreeLibrary(hHidLib);
	}

	hidDeviceObject=CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if(hidDeviceObject == NULL)
        displayError("Error create file");

	return 0;
}
