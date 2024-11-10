#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <ShellAPI.h>
#include <random>
#include <time.h>

#define CALCULATE_LBA(f, MaxLBA, off_set) ((f * (MaxLBA - 1) + off_set <= MaxLBA) ? (f * (MaxLBA - 1)) + off_set : (f * (MaxLBA - 1)) - off_set)

#define READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead) \
    do { \
        LARGE_INTEGER sectorOffset; \
        sectorOffset.QuadPart = lbaAddress * 512; \
        SetFilePointer(hDevice, sectorOffset.LowPart, &sectorOffset.HighPart, FILE_BEGIN); \
        if (!ReadFile(hDevice, buffer, sizeof(buffer), &bytesRead, NULL)) { \
            printf("Failed to read disk\n"); \
            exit(2); \
        } \
    } while(0)

#define BEAT(hDevice, lbaAddress, buffer, bytesRead, off_set) \
	do { \
		lbaAddress = CALCULATE_LBA((InnerSide ? 0 : 1), MaxLBA, off_set); \
		READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead); \
		InnerSide = !InnerSide; \
	} while(0)

// by 1157369
// 编译示例：
// g++.exe ".\src.cpp" -o ".\src.exe" -std=c++11 -I"..\MinGW64\x86_64-w64-mingw32\include" -I"..\MinGW64\lib\gcc\x86_64-w64-mingw32\4.9.2\include" -I"..\MinGW64\lib\gcc\x86_64-w64-mingw32\4.9.2\include\c++" -L"..\MinGW64\lib" -L"..\MinGW64\x86_64-w64-mingw32\lib" -static-libgcc


using namespace std;

bool InnerSide = 1;
long long MaxLBA = -1;
bool echo = 1;
//long long block_size = 32*512;
//long long timeout = 50;
//long long refresh_time_interval = 1000;

// 调试 
//long long start_LBA=00000;
//bool input = 1; 
// char buffer[2146793727];

// 检测是否为管理员身份运行此程序 
bool IsRunAsAdministrator() {
    bool fIsRunAsAdmin = false;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);

        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize)) {
            fIsRunAsAdmin = elevation.TokenIsElevated != 0;
        }
    }

    if (hToken) {
        CloseHandle(hToken);
    }

    return fIsRunAsAdmin;
}

//使用高性能计时器实现的 GetTickCount 函数
__int64 GetTickCountINT() {
	__int64 Freq = 0;
	__int64 Count = 0;
	if(QueryPerformanceFrequency((LARGE_INTEGER*)&Freq)
	        && Freq > 0
	        && QueryPerformanceCounter((LARGE_INTEGER*)&Count)) {
		//乘以1000，把秒化为毫秒
		return Count*1000 / Freq ;
	}
	return -1;
}

DWORDLONG GetMaxLBAForDisk(int diskNumber) {
    HANDLE hDisk = INVALID_HANDLE_VALUE;
    DWORD bytesReturned = 0;
    DISK_GEOMETRY_EX diskGeometry;
    DWORDLONG maxLBA = 0;

    // 打开指定的物理磁盘
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDisk = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        printf("Failed to open device\n");
    	printf("Press enter to exit..."); getchar(); getchar();
        exit(1);
    }

    // 获取物理磁盘的几何信息
    if (!DeviceIoControl(hDisk, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL)) {
        printf("Failed to read disk\n");
        CloseHandle(hDisk);
    	printf("Press enter to exit..."); getchar(); getchar();
        exit(1);
    }

    // 计算最大LBA数字
    maxLBA = diskGeometry.DiskSize.QuadPart / diskGeometry.Geometry.BytesPerSector - 1;

    // 关闭物理磁盘句柄
    CloseHandle(hDisk);

    return maxLBA;
}


int main(){
	printf("By 1157369\n");
    // 检查是否以管理员权限运行
    if (!IsRunAsAdministrator()) {
        // 获取程序路径
        char programPath[MAX_PATH];
        GetModuleFileName(NULL, programPath, MAX_PATH);

        // 请求管理员权限
        if (ShellExecute(NULL, "runas", programPath, NULL, NULL, SW_SHOWNORMAL) <= (HINSTANCE)32) {
        	printf("Unable to get administrator privileges, error code: %d\n", GetLastError());
    		printf("Press enter to exit..."); getchar(); getchar();
            return 1; // 退出程序
        }

        // 标记已请求管理员权限
        return 0;
    } else {
        // 程序主逻辑
        printf("The program has been run with administrator privileges.\n");
        //std::cout << "程序已以管理员权限运行！" << std::endl;
    }

    HANDLE hDevice;
    DWORD bytesRead;
    char buffer[512];
    char devicePath[32];
    long long lbaAddress=0;
    char ifloadfromfile; char score[2048*512]={}; int score_length=0;
	int drvnum, BPM, duration_16th=0;
	long long dTime=0, StartTime=0;
	
	printf("drive number:"); scanf("%d", &drvnum); getchar();
	MaxLBA = GetMaxLBAForDisk(drvnum);
	printf("Max LBA: %I64d\n", MaxLBA);
	
    // 打开物理磁盘
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", drvnum);
    hDevice = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
	{
        printf("Failed to open device\n");
        exit(2);
    }
    
    // 设置随机数发生器 
    //random_device rd;   // non-deterministic generator
    //mt19937_64 gen(rd());  // to seed mersenne twister.
    mt19937_64 gen(time(0));  // to seed mersenne twister.
    uniform_int_distribution<long long> dist(0, MaxLBA/100); // distribute results between 1 and 6 inclusive.
    
	// start 
    BEAT(hDevice, lbaAddress, buffer, bytesRead,dist(gen));
    // for(int i=0; i<100; i++)	BEAT(hDevice, lbaAddress, buffer, bytesRead,dist(gen));
    putchar('\n');
    printf("#  = quarter note		*  = eighth note		-  = sixteenth note\n");
    printf("#. = dotted quarter note	*. = dotted eighth note		-. = dotted sixteenth note\n");
    printf("M  = quarter rest		N  = eighth rest		n  = sixteenth rest\n");
    printf("M. = dotted quarter rest	N. = dotted eighth rest		n. = dotted sixteenth rest\n");
    printf("others = pass\n\n");
    
    printf("load score from score.txt?(Y/N)"); ifloadfromfile=getchar();
    if(ifloadfromfile == 'Y' || ifloadfromfile == 'y')
    {
    	freopen("score.txt","r",stdin);
    	scanf("%d", &BPM); getchar();
    	duration_16th = 60*1000/BPM/4;
    	//scanf("%s", score);
    	gets(score);
    	score_length = strlen(score);
		//freopen("CON", "r", stdin);
	}
    else
    {
    	printf("BPM:"); scanf("%d", &BPM); getchar();
    	duration_16th = 60*1000/BPM/4;
    	printf("score:"); //scanf("%s", score);
    	gets(score);
    	score_length = strlen(score);
	}
	
	printf("go!\n");
	StartTime = GetTickCountINT();
	dTime = 0;
	redo:
	if(GetTickCountINT() - StartTime >= duration_16th/2){
		StartTime = GetTickCountINT();
		printf("Jam happened.\n");
	}
	for(int i=0; i<score_length; i++)
	{
		//CurrentTime = GetTickCountINT();
		switch(score[i])
		{
			case '#':
				BEAT(hDevice, lbaAddress, buffer, bytesRead,dist(gen));
				if(echo)	putchar('#');
				dTime += duration_16th*4;
				break;
			case '*':
				BEAT(hDevice, lbaAddress, buffer, bytesRead,dist(gen));
				if(echo)	putchar('*');
				dTime += duration_16th*2;
				break;
			case '-':
				BEAT(hDevice, lbaAddress, buffer, bytesRead,dist(gen));
				if(echo)	putchar('-');
				dTime += duration_16th*1;
				break;
			case 'M':
				if(echo)	putchar('M');
				dTime += duration_16th*4;
				break;
			case 'N':
				if(echo)	putchar('N');
				dTime += duration_16th*2;
				break;
			case 'n':
				if(echo)	putchar('n');
				dTime += duration_16th*1;
				break;
			case '.':
				if(i==0){
					printf("ERROR: begin with a dot!");
					return 2;
				}
				switch(score[i-1])
				{
					case '#':
						if(echo)	putchar('.');
						dTime += duration_16th*2;
						break;
					case '*':
						if(echo)	putchar('.');
						dTime += duration_16th*1;
						break;
					case '-':
						if(echo)	putchar('.');
						dTime += duration_16th/2;
						break;
					case 'M':
						if(echo)	putchar('.');
						dTime += duration_16th*2;
						break;
					case 'N':
						if(echo)	putchar('.');
						dTime += duration_16th*1;
						break;
					case 'n':
						if(echo)	putchar('.');
						dTime += duration_16th/2;
						break;
					default:
						printf("ERROR: single dot!");
						return 2;
				}
				break;
			default:
				if(echo)	putchar(' ');
				continue;  // legal?
		}
		
		if(GetTickCountINT() - StartTime >= dTime){
			printf("\nJam happened.\n");
			StartTime = GetTickCountINT();
			dTime = 0;
		}
		while(GetTickCountINT() - StartTime < dTime){}
	}
	
	if(ifloadfromfile == 'Y' || ifloadfromfile == 'y')
	{
    	//freopen("score.txt","r+",stdin);
		StartTime = GetTickCountINT();
		dTime = 0;
		if(gets(score)){
			//freopen("CON", "r", stdin);
			score_length = strlen(score);
			putchar('\n');
			goto redo;
		}
		freopen("CON", "r", stdin);
	}
	
    printf("\nPress enter to exit..."); getchar(); //getchar();
	return 0;
}
