#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <sys/mman.h>  
#include <sys/stat.h> 

#include <string.h>
#include<iostream>

//#include"BlockDevice.h"
#include"Buf.h"
#include"BufferManager.h"
#include"file.h"
#include"filesystem.h"
#include"inode.h"
#include"Kernel.h"
#include"openfilemanager.h"
#include"user.h"

using namespace std;


void spb_init(mySuperBlock &sb)
{
	sb.s_isize = myFileSystem::INODE_ZONE_SIZE;
	sb.s_fsize = myFileSystem::DATA_ZONE_END_SECTOR+1;

	//��һ��99�� ��������һ�ٿ�һ�� ʣ�µı�������ֱ�ӹ���
	sb.s_nfree = (myFileSystem::DATA_ZONE_SIZE - 99) % 100;

	//������ֱ�ӹ����Ŀ����̿�ĵ�һ���̿���̿��
	//����������
	int start_last_datablk = myFileSystem::DATA_ZONE_START_SECTOR;
	for (;;)
		if ((start_last_datablk + 100 - 1) < myFileSystem::DATA_ZONE_END_SECTOR)//�ж�ʣ���̿��Ƿ���100��
			start_last_datablk += 100;
		else
			break;
	start_last_datablk--;
	for (int i = 0; i < sb.s_nfree; i++)
		sb.s_free[i] = start_last_datablk + i;

	sb.s_ninode = 100;
	for (int i = 0; i < sb.s_ninode; i++)
		sb.s_inode[i] = i ;//ע������ֻ��diskinode�ı�ţ�����ȡ�õ�ʱ��Ҫ�����̿��ת��

	sb.s_fmod = 0;
	sb.s_ronly = 0;
}

void init_datablock(char *data)
{
	struct {
		int nfree;//������еĸ���
		int free[100];//������е�������
	}tmp_table;

	int last_datablk_num = myFileSystem::DATA_ZONE_SIZE;//δ�����������̿������
	//ע:�������ӷ�,����ĳ�ʼ��������
	for (int i = 0;; i++)
	{
		if (last_datablk_num >= 100)
			tmp_table.nfree = 100;
		else
			tmp_table.nfree = last_datablk_num;
		last_datablk_num -= tmp_table.nfree;

		for (int j = 0; j < tmp_table.nfree; j++)
		{
			if (i == 0 && j == 0)
				tmp_table.free[j] = 0;
			else
			{
				tmp_table.free[j] = 100 * i + j + myFileSystem::DATA_ZONE_START_SECTOR - 1;
			}
		}
		memcpy(&data[99 * 512 + i * 100 * 512], (void*)&tmp_table, sizeof(tmp_table));
		if (last_datablk_num == 0)
			break;
	}
}

int init_img(int fd)
{
	mySuperBlock spb;
	spb_init(spb);
	DiskInode *di = new DiskInode[myFileSystem::INODE_ZONE_SIZE*myFileSystem::INODE_NUMBER_PER_SECTOR];

	//����rootDiskInode�ĳ�ʼֵ
	di[0].d_mode = myInode::IFDIR;
	di[0].d_mode |= myInode::IEXEC;
	//di[0].d_nlink = 888;

	char *datablock = new char[myFileSystem::DATA_ZONE_SIZE * 512];
	memset(datablock, 0, myFileSystem::DATA_ZONE_SIZE * 512);
	init_datablock(datablock);

	write(fd, &spb,  sizeof(mySuperBlock));
	write(fd, di, myFileSystem::INODE_ZONE_SIZE*myFileSystem::INODE_NUMBER_PER_SECTOR * sizeof(DiskInode));
	write(fd, datablock, myFileSystem::DATA_ZONE_SIZE * 512);

	cout << "��ʽ���������" << endl;
//	exit(1);
}

void init_rootInode()
{

}

void sys_init()
{
	cout << "����ϵͳ��ʼ����װ��spb�͸�Ŀ¼inode,����user�ṹ�еı�Ҫ����" << endl;
	myFileManager& fileMgr = myKernel::Instance().GetFileManager();
	fileMgr.rootDirInode = g_InodeTable.IGet(myFileSystem::ROOTINO);
	fileMgr.rootDirInode->i_flag &= (~myInode::ILOCK);

	myKernel::Instance().GetFileSystem().LoadSuperBlock();
	myUser& us = myKernel::Instance().GetUser();
	us.u_cdir = g_InodeTable.IGet(myFileSystem::ROOTINO);
	strcpy(us.u_curdir, "/");
//	Utility::StringCopy("/", us.u_curdir);
	cout << "ϵͳ��ʼ������" << endl;
}



int fcreate(char *filename,int mode)
{
//	printf("\n\n\n--->fcreate ");
	myUser &u = myKernel::Instance().GetUser();
	u.u_ar0 = 0;
	u.u_dirp = filename;
	u.u_arg[1] = myInode::IRWXU;
	myFileManager &fimanag = myKernel::Instance().GetFileManager();
//	printf("�ļ���Ϊ%s\n",filename);
	fimanag.Creat();
//	cout << "�������Ǵ����ɹ���" << endl;
//	cout << u.u_ar0 << endl;
//	printf("fcreate�ɹ�����\n");
	return u.u_ar0;
}

int fopen(char *pathname,int mode)
{
//	cout << "\n\n\n--->fopen" << endl;
	myUser &u = myKernel::Instance().GetUser();
	u.u_ar0 = 0;
	u.u_dirp = pathname;
	u.u_arg[1] = mode;
	myFileManager &fimanag = myKernel::Instance().GetFileManager();
//	printf("�ļ���Ϊ%s\n", pathname);
	fimanag.Open();
//	cout << "Open������ u.u_ar0=" << u.u_ar0 << endl;
	return u.u_ar0;
}

int fwrite(int fd,char *src,int len)
{
//	cout << "\n\n\n--->fwrite" << endl;
	myUser &u = myKernel::Instance().GetUser();
	u.u_ar0 = 0;
	u.u_arg[0] = fd;
	u.u_arg[1] = int(src);
	u.u_arg[2] = len;
	myFileManager &fimanag = myKernel::Instance().GetFileManager();
	fimanag.Write();
//	cout << "Write������ u.u_ar0=" << u.u_ar0 << endl;
	//delete temp;
	return u.u_ar0;
}


int fread(int fd,char *des,int len)
{
//	cout << "\n\n\n--->fread" << endl;
	myUser &u = myKernel::Instance().GetUser();
	u.u_ar0 = 0;
	u.u_arg[0] = fd;
	u.u_arg[1] = int(des);
	u.u_arg[2] = len;
	myFileManager &fimanag = myKernel::Instance().GetFileManager();
	fimanag.Read();
//	cout << "read������ u.u_ar0=" << u.u_ar0 << endl;
	return u.u_ar0;
}

void fdelete(char* name)
{
//	cout << "\n\n\n--->fdelete" << endl;
	myUser &u = myKernel::Instance().GetUser();
	u.u_ar0 = 0;
	u.u_dirp = name;
	myFileManager &fimanag = myKernel::Instance().GetFileManager();
	fimanag.UnLink();
//	cout << "delete������ u.u_ar0=" << u.u_ar0 << endl;
}

int flseek(int fd,int position,int ptrname)
{
//	cout << "\n\n\n--->fseek" << endl;
	myUser &u = myKernel::Instance().GetUser();
	u.u_ar0 = 0;
	u.u_arg[0] = fd;
	u.u_arg[1] = position;
	u.u_arg[2] = ptrname;
	myFileManager &fimanag = myKernel::Instance().GetFileManager();
	fimanag.Seek();
//	cout << "Seek������ u.u_ar0=" << u.u_ar0 << endl;
	return u.u_ar0;
}

void fclose(int fd)
{
//	cout << endl << endl << endl << "--->fclose" << endl;
	myUser &u = myKernel::Instance().GetUser();
	u.u_ar0 = 0;
	myFileManager &fimanag = myKernel::Instance().GetFileManager();
	u.u_arg[0] = fd;
	fimanag.Close();
//	cout << "delete������ u.u_ar0=" << u.u_ar0 << endl;
}


void ls()
{
	myUser &u = myKernel::Instance().GetUser();
//	strcpy(u.u_dirp, u.u_curdir);
//	u.u_arg[1] = (myFile::FREAD) | (myFile::FWRITE);
	int fd = fopen(u.u_curdir, (myFile::FREAD) );
	char temp_filename[32] = { 0 };
	for (;;)
	{
		if (fread(fd, temp_filename, 32) == 0) {
	//		cout << "ls�ɹ�����" << endl;
			return;
		}
		else
		{
			//for (int i = 0; i < 32; i++)
			//	cout << temp_filename[i] << "  " << int(temp_filename) << endl;
			myDirectoryEntry *mm = (myDirectoryEntry*)temp_filename;
			cout << "======" << mm->m_name << "======" << endl;
			memset(temp_filename, 0, 32);
		}
	}
	
}
void quitOS(char *addr,int len)
{
	myBufferManager &bm = myKernel::Instance().GetBufferManager();
	bm.Bflush();
	msync(addr, len, MS_SYNC);
	myInodeTable *mit = myKernel::Instance().GetFileManager().m_InodeTable;
	mit->UpdateInodeTable();
}


int main()
{
	//int fd;
	/*���ļ�*/
	int fd = open("c.img", O_RDWR);
	if (fd == -1) {//�ļ�������  
		fd = open("c.img", O_RDWR | O_CREAT, 0666);
		if (fd == -1) {
			printf("�򿪻򴴽��ļ�ʧ��:%m\n");
			exit(-1);
		}
		init_img(fd);
	}
	struct stat st; //�����ļ���Ϣ�ṹ��  
					/*ȡ���ļ���С*/
	int r = fstat(fd, &st);
	if (r == -1) {
		printf("��ȡ�ļ���Сʧ��:%m\n");
		close(fd);
		exit(-1);
	}
	int len = st.st_size;
	/*���ļ�ӳ��������ڴ��ַ*/
	void * addr=mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	myKernel::Instance().Initialize((char*)addr);

	sys_init();



	cout << "                    ��UNIX V6++�����ļ�ϵͳʵ��                     " << endl;
	cout << "                             *******                                " << endl;
	cout << "                         Welcome to Linux World                     " << endl;
	cout << "           CopyRight @ Xuesen Huang Tongji University  2018         " << endl;
	cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
	while (1)
	{
		char WhichToDo=-1;
		cout << "===============================================================" << endl;
		cout << "||��������Ҫִ�е�API�Ķ�Ӧ��ţ�������ʾ��                  ||" << endl;
		cout << "||1   fopen(char *name, int mode)                            ||" << endl;
		cout << "||2   fclose(int fd)                                         ||" << endl;
		cout << "||3   fread(int fd, int length)                              ||" << endl;
		cout << "||4   fwrite(int fd, char *buffer, int length)               ||" << endl;
		cout << "||5   flseek(int fd, int position, int ptrname)              ||" << endl;
		cout << "||6   fcreat(char *name, int mode)                           ||" << endl;
		cout << "||7   fdelete(char *name)                                    ||" << endl;
		cout << "||8   ls()                                                   ||" << endl;
		cout << "||q  �˳��ļ�ϵͳ                                            ||" << endl << endl << endl;
		//cout << "===============================================================" << endl;
		cout << "||SecondFileSystem@ ��������>>";
		cin >> WhichToDo;
		string filename;
		string inBuf;
		int temp_fd; 
		int outSeek;
		int inLen;
		int mode;
		int openfd;
		int temp_ptrname;
		int outLen;
		int temp_position;
		int readNum;
		int creatfd;
		int writeNum = 0;
		char c;
		char *temp_inBuf, *temp_des, *temp_filename;
		switch (WhichToDo)
		{
		case '1'://fopen
			cout << "||SecondFileSystem@ �������һ�������ļ���>>";
			cin >> filename;
			temp_filename = new char[filename.length()+1];
			strcpy(temp_filename, filename.c_str());
			cout << "||SecondFileSystem@ ������ڶ����������򿪷�ʽ>>";
			cin >> mode;
			openfd = fopen(temp_filename, mode);
			if (openfd < 0)
				cout << "||SecondFileSystem@ openʧ��" << endl;
			else
				cout << "||SecondFileSystem@ open ����fd=" << openfd << endl;
			delete temp_filename;
			break;
		case '2'://fclose
			cout << "||SecondFileSystem@ �������һ�������ļ����>>";
			cin >> temp_fd;
			fclose(temp_fd);
			break;
		case '3'://fread
			cout << "||SecondFileSystem@ �������һ���������ļ����>>";
			cin >> temp_fd;
			cout << "||SecondFileSystem@ ������ڶ������������������ݳ���:";
			cin >>outLen;
			temp_des = new char[1+outLen];
			memset(temp_des, 0, outLen + 1);
			readNum = fread(temp_fd, temp_des, outLen);
			cout << "||SecondFileSystem@ read����" << readNum << endl;
			cout << "||SecondFileSystem@ ��������Ϊ:" << endl;
			cout << temp_des << endl;
			break;
		case '4'://fwrite
			cout << "||SecondFileSystem@ �������һ�������ļ����>>";
			cin >> temp_fd;
			cout << "||SecondFileSystem@ ������ڶ���������д������>>";
			cin >> inBuf;
			temp_inBuf = new char[inBuf.length() + 1];
			strcpy(temp_inBuf, inBuf.c_str());
			cout << "||SecondFileSystem@ �����������������д�����ݵĳ���>>";
			cin >>inLen;
			writeNum = fwrite(temp_fd, temp_inBuf, inLen);
			cout << "||SecondFileSystem@ д����Ϊ" << writeNum << endl;
			break;
		case '5'://flseek
			cout << "||SecondFileSystem@ �������һ�������ļ����>>";
			cin >> temp_fd;
			cout << "||SecondFileSystem@ ������ڶ����������ƶ�λ��>>";
			cin >> temp_position;
			cout << "||SecondFileSystem@ ������������������ƶ���ʽ>>";
			cin >> temp_ptrname;
			outSeek = flseek(temp_fd, temp_position, temp_ptrname);
			cout << "||SecondFileSystem@ fseek��������" << outSeek << endl;
			break;
		case '6'://fcreat
			cout << "||SecondFileSystem@ �������һ�������ļ���>>";
			cin >> filename;
			temp_filename = new char[filename.length() + 1];
			strcpy(temp_filename, filename.c_str());
			cout << "||SecondFileSystem@ ������ڶ���������������ʽ>>";
			cin >> mode;
			creatfd = fcreate(temp_filename, mode);
			if (creatfd < 0)
				cout << "||SecondFileSystem@ createʧ��" << endl;
			else
				cout << "create�ɹ� ����fd=" << creatfd << endl;
			delete temp_filename;
			break;
		case '7'://fdelete
			cout << "||SecondFileSystem@ �������һ�������ļ���>>";
			cin >> filename;
			temp_filename = new char[filename.length() + 1];
			strcpy(temp_filename, filename.c_str());
			fdelete(temp_filename);
			delete temp_filename;
			break;
		case '8':
			cout << "||SecondFileSystem@ �������>>" << endl;
			ls();
			break;

		case 'q':
			quitOS((char*)addr,len);
			return 1;
			break;
		default:
			cout << "||SecondFileSystem@ ���벻�Ϸ�������������" << endl;
			while ((c = getchar()) != EOF && c != '\n');
			break;
		}
	}



	/*
	char tempBuf[32] = { 0 };
	int ffd=fcreate("text.txt",myInode::IRWXU);
	fwrite(ffd,"hello,world",12);
	int fffd = fopen("text.txt", (myFile::FREAD) | (myFile::FWRITE));
	fseek(fffd,6,0);
	fread(fffd,tempBuf,5);
	cout << tempBuf << endl;
	ls();
	fdelete("text.txt");
	fopen("text.txt", (myFile::FREAD) | (myFile::FWRITE));
	*/
}