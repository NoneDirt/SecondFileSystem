

#include"openfilemanager.h"
#include"user.h"
#include"Kernel.h"


#include<iostream>
#include <stdio.h>
using namespace std;
//ע��ȫ��superblockʵ��
mySuperBlock g_spb;
/*==============================class OpenFileTable===================================*/
/* ϵͳȫ�ִ��ļ������ʵ���Ķ��� */
myOpenFileTable g_OpenFileTable;

myOpenFileTable::myOpenFileTable()
{
	//nothing to do here
}

myOpenFileTable::~myOpenFileTable()
{
	//nothing to do here
}

/*���ã����̴��ļ������������ҵĿ�����  ֮ �±�  д�� u_ar0[EAX]*/
myFile* myOpenFileTable::FAlloc()
{
	//cout << "OpenFileTable.FAlloc" << endl;
	int fd;
	myUser& u = myKernel::Instance().GetUser();

	/* �ڽ��̴��ļ����������л�ȡһ�������� */
	fd = u.u_ofiles.AllocFreeSlot();

	if (fd < 0)	/* ���Ѱ�ҿ�����ʧ�� */
	{
		//cout << "δѰ�ҵ�����File�� FAlloc����NULL" << endl;
		return NULL;
	}

	for (int i = 0; i < myOpenFileTable::NFILE; i++)
	{
		/* f_count==0��ʾ������� */
		if (this->m_File[i].f_count == 0)
		{
			/* ������������File�ṹ�Ĺ�����ϵ */
			u.u_ofiles.SetF(fd, &this->m_File[i]);
			/* ���Ӷ�file�ṹ�����ü��� */
			this->m_File[i].f_count++;
			/* ����ļ�����дλ�� */
			this->m_File[i].f_offset = 0;
			//cout << "FAlloc ��ȷ���ض�Ӧ�����File��" << endl;
			return (&this->m_File[i]);
		}
	}
	printf("No Free File Struct\n");
	//Diagnose::Write("No Free File Struct\n");
	u.u_error = myUser::my_ENFILE;
	return NULL;
}


void myOpenFileTable::CloseF(myFile *pFile)
{
	myInode* pNode;

	//ע�������ǹܵ�����

	if (pFile->f_count <= 1)
	{
		/*
		* �����ǰ���������һ�����ø��ļ��Ľ��̣�
		* ��������豸���ַ��豸�ļ�������Ӧ�Ĺرպ���
		*/
		//ע������Ҫ�����豸�Ĺرպ������ͷ���Ӧinode����
	//	pFile->f_inode->CloseI(pFile->f_flag & File::FWRITE);
		g_InodeTable.IPut(pFile->f_inode);
	}

	/* ���õ�ǰFile�Ľ�������1 */
	pFile->f_count--;
}


/*==============================class myInodeTable===================================*/
/*  �����ڴ�Inode���ʵ�� */
myInodeTable g_InodeTable;

myInodeTable::myInodeTable()
{
	//nothing to do here
}

myInodeTable::~myInodeTable()
{
	//nothing to do here
}

void myInodeTable::Initialize()
{
	/* ��ȡ��g_FileSystem������ */
	this->m_FileSystem = &myKernel::Instance().GetFileSystem();
}

myInode* myInodeTable::IGet( int inumber)
{

	//cout << "InodeTable.IGet" << endl;
	myInode* pInode;
	myUser& u = myKernel::Instance().GetUser();

	while (true)
	{
		/* ���ָ���豸dev�б��Ϊinumber�����Inode�Ƿ����ڴ濽�� */
		int index = this->IsLoaded( inumber);
		if (index >= 0)	/* �ҵ��ڴ濽�� */
		{
			pInode = &(this->m_Inode[index]);
			/* ������ڴ�Inode������ */
			//ע����������

			/* ������ڴ�Inode�����������ļ�ϵͳ�����Ҹ�Inode��Ӧ��Mountװ��� */
			//ע�����������ļ�ϵͳ�����

			/*
			* ����ִ�е������ʾ���ڴ�Inode���ٻ������ҵ���Ӧ�ڴ�Inode��
			* ���������ü���������ILOCK��־������֮
			*/
			pInode->i_count++;
			//cout << "��inode�Ѿ���loaded���᷵�ظý�� ��inodeΪ"<<inumber <<"Iget��������"<< endl;
			return pInode;
		}
		else	/* û��Inode���ڴ濽���������һ�������ڴ�Inode */
		{
			pInode = this->GetFreeInode();
			/* ���ڴ�Inode���������������Inodeʧ�� */
			if (NULL == pInode)
			{
				printf("Inode Table Overflow !\n");
				//Diagnose::Write("Inode Table Overflow !\n");
				u.u_error = myUser::my_ENFILE;
				return NULL;
			}
			else	/* �������Inode�ɹ��������Inode�����·�����ڴ�Inode */
			{
				/* �����µ��豸�š����Inode��ţ��������ü������������ڵ����� */
				//pInode->i_dev = dev;
				pInode->i_number = inumber;
			//	cout << "�ɹ���ÿ���inode pInode->i_number=" << pInode->i_number << endl;
				pInode->i_count++;
				pInode->i_lastr = -1;

				myBufferManager& bm = myKernel::Instance().GetBufferManager();
				/* �������Inode���뻺���� */
				myBuf* pBuf = bm.Bread(myFileSystem::INODE_ZONE_START_SECTOR + inumber / myFileSystem::INODE_NUMBER_PER_SECTOR);

				/* �������I/O���� */
				if (pBuf->b_flags & myBuf::B_ERROR)
				{
					cout << "������������ͷŻ��棬IGet�����󷵻�" << endl;
					/* �ͷŻ��� */
					bm.Brelse(pBuf);
					/* �ͷ�ռ�ݵ��ڴ�Inode */
					this->IPut(pInode);
					return NULL;
				}

				/* ���������е����Inode��Ϣ�������·�����ڴ�Inode�� */
				pInode->ICopy(pBuf, inumber);
				/* �ͷŻ��� */
				bm.Brelse(pBuf);
			//	cout << "�ͷ���buf��Iget���ɹ�����" << endl;
				return pInode;
			}
		}
	}
	return NULL;	/* GCC likes it! */
}

/* close�ļ�ʱ�����Iput
*      ��Ҫ���Ĳ������ڴ�i�ڵ���� i_count--����Ϊ0���ͷ��ڴ� i�ڵ㡢���иĶ�д�ش���
* �����ļ�;��������Ŀ¼�ļ������������󶼻�Iput���ڴ�i�ڵ㡣·�����ĵ�����2��·������һ���Ǹ�
*   Ŀ¼�ļ�������������д������ļ�������ɾ��һ�������ļ���������������д���ɾ����Ŀ¼����ô
*   	���뽫���Ŀ¼�ļ�����Ӧ���ڴ� i�ڵ�д�ش��̡�
*   	���Ŀ¼�ļ������Ƿ��������ģ����Ǳ��뽫����i�ڵ�д�ش��̡�
* */
void myInodeTable::IPut(myInode *pNode)
{
	//cout << "Inodetable.IPut" << endl;
	/* ��ǰ����Ϊ���ø��ڴ�Inode��Ψһ���̣���׼���ͷŸ��ڴ�Inode */
	if (pNode->i_count == 1)
	{

		/* ���ļ��Ѿ�û��Ŀ¼·��ָ���� */
		if (pNode->i_nlink <= 0)
		{
			/* �ͷŸ��ļ�ռ�ݵ������̿� */
			pNode->ITrunc();
			pNode->i_mode = 0;
			/* �ͷŶ�Ӧ�����Inode */
			this->m_FileSystem->IFree(pNode->i_number);
		}

		/* �������Inode��Ϣ */
		pNode->IUpdate();

		/* ����ڴ�Inode�����б�־λ */
		pNode->i_flag = 0;
		/* �����ڴ�inode���еı�־֮һ����һ����i_count == 0 */
		pNode->i_number = -1;
	}

	/* �����ڴ�Inode�����ü��������ѵȴ����� */
	pNode->i_count--;
}

void myInodeTable::UpdateInodeTable()
{
	for (int i = 0; i < myInodeTable::NINODE; i++)
	{
		/*
		* ���Inode����û�б�����������ǰδ����������ʹ�ã�����ͬ�������Inode��
		* ����count������0��count == 0��ζ�Ÿ��ڴ�Inodeδ���κδ��ļ����ã�����ͬ����
		*/
		if ( this->m_Inode[i].i_count != 0)
		{
			/* ���ڴ�Inode������ͬ�������Inode */
			this->m_Inode[i].i_flag |= myInode::ILOCK;
			this->m_Inode[i].IUpdate();
		}
	}
}

int myInodeTable::IsLoaded( int inumber)
{
	//cout << "InodeTable.IsLoaded" << endl;
	/* Ѱ��ָ�����Inode���ڴ濽�� */
	for (int i = 0; i < myInodeTable::NINODE; i++)
	{
		if ( this->m_Inode[i].i_number == inumber && this->m_Inode[i].i_count != 0)
		{
		//	cout << "�ҵ���Ӧ�ڴ�inode�ڵ㣬���Ϊ" <<i<< endl;
			return i;
		}
	}
//	cout << "û�ҵ���Ӧ�ڴ�inode�ڵ�"<< endl;
	return -1;
}

myInode* myInodeTable::GetFreeInode()
{
	//cout << "in GetFreeInode" << endl;
	for (int i = 0; i < myInodeTable::NINODE; i++)
	{
		/* ������ڴ�Inode���ü���Ϊ�㣬���Inode��ʾ���� */
		if (this->m_Inode[i].i_count == 0)
		{
		//	cout << "�����ؿ����ڴ�inode��ַ" << endl;
			return &(this->m_Inode[i]);
		}
	}
	cout << "Ѱ�ҿ����ڴ�inodeʧ�ܷ���NULL" << endl;
	return NULL;	/* Ѱ��ʧ�� */
}
