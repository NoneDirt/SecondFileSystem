

#include"file.h"
#include"user.h"
#include"Kernel.h"
#include<iostream>
#include <stdio.h>

using namespace std;

/*==============================class File===================================*/
myFile::myFile()
{
	this->f_count = 0;
	this->f_flag = 0;
	this->f_offset = 0;
	this->f_inode = NULL;
}

myFile::~myFile()
{
	//nothing to do here
}

/*==============================class OpenFiles===================================*/
myOpenFiles::myOpenFiles()
{
}

myOpenFiles::~myOpenFiles()
{
}

int myOpenFiles::AllocFreeSlot()
{
//	cout << "OpenFile.AllicFreeSlot" << endl;
	int i;
	myUser& u = myKernel::Instance().GetUser();

	for (i = 0; i < myOpenFiles::NOFILES; i++)
	{
		/* ���̴��ļ������������ҵ�������򷵻�֮ */
		if (this->ProcessOpenFileTable[i] == NULL)
		{
			/* ���ú���ջ�ֳ��������е�EAX�Ĵ�����ֵ����ϵͳ���÷���ֵ */
	//		cout << "���䵽��Ӧ��File�ṹ������FIle���������Ϊ i=" << i << " ����u.u_ar0Ϊi  AllocFreeSlot�ɹ�����" << endl;
			u.u_ar0 = i;
			return i;
		}
	}
//	cout << "δ���䵽���е�File�飬AllocFreeSlot���󷵻�-1��u.u_ar0=-1" << endl;
	u.u_ar0 = -1;   /* Open1����Ҫһ����־�������ļ��ṹ����ʧ��ʱ�����Ի���ϵͳ��Դ*/
	u.u_error = myUser::my_EMFILE;
	return -1;
}


myFile* myOpenFiles::GetF(int fd)
{
//	cout << "OpenFiles.GetF fd=" << fd << endl;
	myFile* pFile;
	myUser& u = myKernel::Instance().GetUser();

	/* ������ļ���������ֵ�����˷�Χ */
	if (fd < 0 || fd >= myOpenFiles::NOFILES)
	{
		u.u_error = myUser::my_EBADF;
		return NULL;
	}

	pFile = this->ProcessOpenFileTable[fd];
	if (pFile == NULL)
	{
		cout << "û���ҵ�fd��Ӧ��File�ṹ,GetF���󷵻�" << endl;
		u.u_error = myUser::my_EBADF;
	}
//	cout << "GetF ��������" << endl;
	return pFile;	/* ��ʹpFile==NULLҲ���������ɵ���GetF�ĺ������жϷ���ֵ */
}


void myOpenFiles::SetF(int fd, myFile* pFile)
{
	//cout << "OpenFiles.SetF" << endl;
	if (fd < 0 || fd >= myOpenFiles::NOFILES)
	{
		cout << "��������SetF���󷵻�" << endl;
		return;
	}
	/* ���̴��ļ�������ָ��ϵͳ���ļ�������Ӧ��File�ṹ */
//	cout << "�ɹ�����fd��File��Ĺ�����ϵ��SetF��ȷ����" << endl;
	this->ProcessOpenFileTable[fd] = pFile;
}

/*==============================class IOParameter===================================*/
IOParameter::IOParameter()
{
	this->m_Base = 0;
	this->m_Count = 0;
	this->m_Offset = 0;
}

IOParameter::~IOParameter()
{
	//nothing to do here
}

