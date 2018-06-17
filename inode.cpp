

#include"inode.h"
#include"BufferManager.h"
#include "Buf.h"
#include"Kernel.h"
#include<algorithm>
#include<string.h>
#include<iostream>
#include <stdio.h>
using namespace std;





//======================================================INODE=========================================================//




/* �ڴ�� i�ڵ�*/
myInode::myInode()
{
	/* ���Inode�����е����� */
	// this->Clean(); 
	/* ȥ��this->Clean();�����ɣ�
	* myInode::Clean()�ض�����IAlloc()������·���DiskInode��ԭ�����ݣ�
	* �����ļ���Ϣ��Clean()�����в�Ӧ�����i_dev, i_number, i_flag, i_count,
	* ���������ڴ�Inode����DiskInode�����ľ��ļ���Ϣ����Inode�๹�캯����Ҫ
	* �����ʼ��Ϊ��Чֵ��
	*/

	/* ��Inode����ĳ�Ա������ʼ��Ϊ��Чֵ */
	this->i_flag = 0;
	this->i_mode = 0;
	this->i_count = 0;
	this->i_nlink = 0;
	this->i_dev = -1;
	this->i_number = -1;
	this->i_uid = -1;
	this->i_gid = -1;
	this->i_size = 0;
	this->i_lastr = -1;
	for (int i = 0; i < 10; i++)
	{
		this->i_addr[i] = 0;
	}
}



myInode::~myInode()
{
	//nothing to do here
}


void myInode::ReadI()
{
	int lbn;	/* �ļ��߼���� */
	int bn;		/* lbn��Ӧ�������̿�� */
	int offset;	/* ��ǰ�ַ�������ʼ����λ�� */
	int nbytes;	/* �������û�Ŀ�����ֽ����� */
	short dev;
	myBuf* pBuf;
	myUser& u = myKernel::Instance().GetUser();
	myBufferManager& bufMgr = myKernel::Instance().GetBufferManager();

	if (0 == u.u_IOParam.m_Count)
	{
		/* ��Ҫ���ֽ���Ϊ�㣬�򷵻� */
		return;
	}

	this->i_flag |= myInode::IACC;

	/* һ��һ���ַ���ض�������ȫ�����ݣ�ֱ�������ļ�β */
	while (myUser::my_NOERROR == u.u_error && u.u_IOParam.m_Count != 0)
	{
		lbn = bn = u.u_IOParam.m_Offset / myInode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % myInode::BLOCK_SIZE;
		/* ���͵��û������ֽ�������ȡ�������ʣ���ֽ����뵱ǰ�ַ�������Ч�ֽ�����Сֵ */
		nbytes = (myInode::BLOCK_SIZE - offset) < /* ������Ч�ֽ��� */u.u_IOParam.m_Count ? (myInode::BLOCK_SIZE - offset) : u.u_IOParam.m_Count;

		if ((this->i_mode & myInode::IFMT) != myInode::IFBLK)
		{	/* �������������豸�ļ� */

			int remain = this->i_size - u.u_IOParam.m_Offset;
			/* ����Ѷ��������ļ���β */
			if (remain <= 0)
			{
				return;
			}
			/* ���͵��ֽ�������ȡ����ʣ���ļ��ĳ��� */
			nbytes = (nbytes < remain) ? nbytes : remain;

			/* ���߼����lbnת���������̿��bn ��Bmap������Inode::rablock����UNIX��Ϊ��ȡԤ����Ŀ���̫��ʱ��
			* �����Ԥ������ʱ myInode::rablock ֵΪ 0��
			* */
			if ((bn = this->Bmap(lbn)) == 0)
			{
				return;
			}
			dev = this->i_dev;
		}
		else	/* �����������豸�ļ� */ //ע����Զ�����ܵ���
		{
			dev = this->i_addr[0];	/* ������豸�ļ�i_addr[0]�д�ŵ����豸�� */
			//myInode::rablock = bn + 1;
		}

		//ע����������û����Ԥ������������û��ʱ����
	
		pBuf = bufMgr.Bread(bn);
		
		/* ��¼�����ȡ�ַ�����߼���� */
		this->i_lastr = lbn;

		/* ������������ʼ��λ�� */
		unsigned char* start = pBuf->b_addr + offset;

		//ע��ֱ��memcpy�ͺ�
		/* ������: �ӻ������������û�Ŀ����
		* i386оƬ��ͬһ��ҳ��ӳ���û��ռ���ں˿ռ䣬��һ��Ӳ���ϵĲ��� ʹ��i386��ʵ�� iomove����
		* ��PDP-11Ҫ�������*/
		//cout << "���ζ���nbytes=" << nbytes <<"�ֽ�"<< endl;
		memcpy(u.u_IOParam.m_Base, start, nbytes);
		//Utility::IOMove(start, u.u_IOParam.m_Base, nbytes);

		/* �ô����ֽ���nbytes���¶�дλ�� */
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		bufMgr.Brelse(pBuf);	/* ʹ���껺�棬�ͷŸ���Դ */
	}
	//cout << "ReadI����" << endl;
}


void myInode::WriteI()
{
	//cout << "Inode.WriteI" << endl;
	int lbn;	/* �ļ��߼���� */
	int bn;		/* lbn��Ӧ�������̿�� */
	int offset;	/* ��ǰ�ַ�������ʼ����λ�� */
	int nbytes;	/* �����ֽ����� */
	short dev;
	myBuf* pBuf;
	myUser& u = myKernel::Instance().GetUser();
	myBufferManager& bufMgr = myKernel::Instance().GetBufferManager();

	/* ����Inode�����ʱ�־λ */
	this->i_flag |= (myInode::IACC | myInode::IUPD);

	if (0 == u.u_IOParam.m_Count)
	{
		//cout << "��Ҫ��д���ֽ���Ϊ0��WriteIֱ�ӷ���" << endl;
		/* ��Ҫ���ֽ���Ϊ�㣬�򷵻� */
		return;
	}
	//cout << "��Ҫд����ֽ���Ϊ  u.u_IOParam.m_Count=" << u.u_IOParam.m_Count << endl;
	while (myUser::my_NOERROR == u.u_error && u.u_IOParam.m_Count != 0)
	{
		lbn = u.u_IOParam.m_Offset / myInode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % myInode::BLOCK_SIZE;
		nbytes = (myInode::BLOCK_SIZE - offset) < u.u_IOParam.m_Count ? (myInode::BLOCK_SIZE - offset) : u.u_IOParam.m_Count;

		if ((this->i_mode & myInode::IFMT) != myInode::IFBLK)
		{	/* ��ͨ�ļ� */
			//cout << "����д����ͨ�ļ�" << endl;
			/* ���߼����lbnת���������̿��bn */
			if ((bn = this->Bmap(lbn)) == 0)
			{
				return;
			}
			dev = this->i_dev;
		}
		else
		{	/* ���豸�ļ���Ҳ����Ӳ�� */
			//cout << "����д����豸�ļ���" << endl;
			dev = this->i_addr[0];
		}

		if (myInode::BLOCK_SIZE == nbytes)
		{
			/* ���д������������һ���ַ��飬��Ϊ����仺�� */
			pBuf = bufMgr.GetBlk( bn);
		}
		else
		{
			//cout << "����д�벻��һ���ַ��齫����bread���ȶ���д  �����̿����bn="<<bn << endl;
			/* д�����ݲ���һ���ַ��飬�ȶ���д���������ַ����Ա�������Ҫ��д�����ݣ� */
			pBuf = bufMgr.Bread( bn);
		}

		/* ���������ݵ���ʼдλ�� */
		//cout << "�������ݵ���ʼдλ��=pBuf->b_addr + offset  offset=" << offset << endl;
		unsigned char* start = pBuf->b_addr + offset;

		/* д����: ���û�Ŀ�����������ݵ������� */
		//cout << "д������Ϊ" << &u.u_IOParam.m_Base[4] <<"  nbytes="<<nbytes<< endl;
		memcpy(start, u.u_IOParam.m_Base, nbytes);
		//Utility::IOMove(u.u_IOParam.m_Base, start, nbytes);

		/* �ô����ֽ���nbytes���¶�дλ�� */
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		if (u.u_error != myUser::my_NOERROR)	/* д�����г��� */
		{
			cout << "writeI�����г���,�ͷŻ���" << endl;
			bufMgr.Brelse(pBuf);
		}
		else if ((u.u_IOParam.m_Offset % myInode::BLOCK_SIZE) == 0)	/* ���д��һ���ַ��� */
		{
			/* ���첽��ʽ���ַ���д����̣����̲���ȴ�I/O�������������Լ�������ִ�� */
			bufMgr.Bawrite(pBuf);
		}
		else /* ���������δд�� */
		{
			/* ��������Ϊ�ӳ�д�������ڽ���I/O�������ַ�������������� */
			//cout << "ע�⣺writeI��ǵ����ӳ�д" << endl;
			bufMgr.Bdwrite(pBuf);
		}

		/* ��ͨ�ļ��������� */
		if ((this->i_size < u.u_IOParam.m_Offset) && (this->i_mode & (myInode::IFBLK & myInode::IFCHR)) == 0)
		{
			this->i_size = u.u_IOParam.m_Offset;
			//cout << "�ļ��������ӣ�����Ϊi_size=" << i_size << endl;
		}

		/*
		* ֮ǰ�����ж��̿��ܵ��½����л����ڽ���˯���ڼ䵱ǰ�ڴ�Inode����
		* ��ͬ�������Inode���ڴ���Ҫ�������ø��±�־λ��
		* ����û�б�Ҫѽ����ʹwriteϵͳ����û��������iput����i_count����0֮��ŻὫ�ڴ�i�ڵ�ͬ���ش��̡�������
		* �ļ�û��close֮ǰ�ǲ��ᷢ���ġ�
		* ���ǵ�ϵͳ��writeϵͳ���������͸������ܳ�����������ˡ�
		* ��������ȥ����
		*/
		this->i_flag |= myInode::IUPD;
	}
	//cout << "WriteI����" << endl;
}


int myInode::Bmap(int lbn)
{
	//cout << "Inode.Bmap" << endl;
	myBuf* pFirstBuf;
	myBuf* pSecondBuf;
	int phyBlkno;	/* ת����������̿�� */
	int* iTable;	/* ���ڷ��������̿���һ�μ�ӡ����μ�������� */
	int index;
	myUser& u = myKernel::Instance().GetUser();
	myBufferManager& bufMgr = myKernel::Instance().GetBufferManager();
	myFileSystem& fileSys = myKernel::Instance().GetFileSystem();

	/*
	* Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
	* (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
	*
	* (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
	* �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
	*
	* (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
	* �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
	* (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	*/

	if (lbn >= myInode::HUGE_FILE_BLOCK)
	{
		u.u_error = myUser::my_EFBIG;
		return 0;
	}

	if (lbn < 6)		/* �����С���ļ����ӻ���������i_addr[0-5]�л�������̿�ż��� */
	{
		phyBlkno = this->i_addr[lbn];

		/*
		* ������߼���Ż�û����Ӧ�������̿����֮��Ӧ�������һ������顣
		* ��ͨ�������ڶ��ļ���д�룬��д��λ�ó����ļ���С�����Ե�ǰ
		* �ļ���������д�룬����Ҫ�������Ĵ��̿飬��Ϊ֮�����߼����
		* �������̿��֮���ӳ�䡣
		*/
		if (phyBlkno == 0 && (pFirstBuf = fileSys.Alloc()) != NULL)
		{
		//	cout << "�����̿��Ϊ0��û�ж�Ӧ�ļ�����ʱ��������еĴ������,�õ��Ĵ��̿��Ϊ pFirstBuf->b_blkno=" << pFirstBuf->b_blkno<< endl;
			/*
			* ��Ϊ����ܿ������ϻ�Ҫ�õ��˴��·�������ݿ飬���Բ��������������
			* �����ϣ����ǽ�������Ϊ�ӳ�д��ʽ���������Լ���ϵͳ��I/O������
			*/
			bufMgr.Bdwrite(pFirstBuf);
			phyBlkno = pFirstBuf->b_blkno;
			/* ���߼����lbnӳ�䵽�����̿��phyBlkno */
			this->i_addr[lbn] = phyBlkno;
			this->i_flag |= myInode::IUPD;
		}
		/* �ҵ�Ԥ�����Ӧ�������̿�� */
		//myInode::rablock = 0;
		if (lbn <= 4)
		{
			/*
			* i_addr[0] - i_addr[5]Ϊֱ�����������Ԥ�����Ӧ�����ſ��Դ�
			* ֱ���������л�ã����¼��Inode::rablock�С������Ҫ�����I/O����
			* �����������飬���Եò�ֵ̫���ˡ�Ư����
			*/
			//ע��Ư��Ҳû�� ��û��Ԥ��
		//	myInode::rablock = this->i_addr[lbn + 1];
		}
	//	cout << "ӳ��������̿��Ϊ phyBlkno=" << phyBlkno << "  Bmanp����" << endl;
		return phyBlkno;
	}
	else	/* lbn >= 6 ���͡������ļ� */
	{
		/* �����߼����lbn��Ӧi_addr[]�е����� */

		if (lbn < myInode::LARGE_FILE_BLOCK)	/* �����ļ�: ���Ƚ���7 - (128 * 2 + 6)���̿�֮�� */
		{
			index = (lbn - myInode::SMALL_FILE_BLOCK) / myInode::ADDRESS_PER_INDEX_BLOCK + 6;
		}
		else	/* �����ļ�: ���Ƚ���263 - (128 * 128 * 2 + 128 * 2 + 6)���̿�֮�� */
		{
			index = (lbn - myInode::LARGE_FILE_BLOCK) / (myInode::ADDRESS_PER_INDEX_BLOCK * myInode::ADDRESS_PER_INDEX_BLOCK) + 8;
		}

		phyBlkno = this->i_addr[index];
		/* ������Ϊ�㣬���ʾ��������Ӧ�ļ��������� */
		if (0 == phyBlkno)
		{
			this->i_flag |= myInode::IUPD;
			/* ����һ�����̿��ż�������� */
			if ((pFirstBuf = fileSys.Alloc()) == NULL)
			{
				return 0;	/* ����ʧ�� */
			}
			/* i_addr[index]�м�¼���������������̿�� */
			this->i_addr[index] = pFirstBuf->b_blkno;
		}
		else
		{
			/* �����洢�����������ַ��� */
			pFirstBuf = bufMgr.Bread( phyBlkno);
		}
		/* ��ȡ��������ַ */
		iTable = (int *)pFirstBuf->b_addr;

		if (index >= 8)	/* ASSERT: 8 <= index <= 9 */
		{
			/*
			* ���ھ����ļ��������pFirstBuf���Ƕ��μ��������
			* ��������߼���ţ����ɶ��μ���������ҵ�һ�μ��������
			*/
			index = ((lbn - myInode::LARGE_FILE_BLOCK) / myInode::ADDRESS_PER_INDEX_BLOCK) % myInode::ADDRESS_PER_INDEX_BLOCK;

			/* iTableָ�򻺴��еĶ��μ������������Ϊ�㣬������һ�μ�������� */
			phyBlkno = iTable[index];
			if (0 == phyBlkno)
			{
				if ((pSecondBuf = fileSys.Alloc()) == NULL)
				{
					/* ����һ�μ����������̿�ʧ�ܣ��ͷŻ����еĶ��μ��������Ȼ�󷵻� */
					bufMgr.Brelse(pFirstBuf);
					return 0;
				}
				/* ���·����һ�μ����������̿�ţ�������μ����������Ӧ�� */
				iTable[index] = pSecondBuf->b_blkno;
				/* �����ĺ�Ķ��μ���������ӳ�д��ʽ��������� */
				bufMgr.Bdwrite(pFirstBuf);
			}
			else
			{
				/* �ͷŶ��μ��������ռ�õĻ��棬������һ�μ�������� */
				bufMgr.Brelse(pFirstBuf);
				pSecondBuf = bufMgr.Bread( phyBlkno);
			}

			pFirstBuf = pSecondBuf;
			/* ��iTableָ��һ�μ�������� */
			iTable = (int *)pSecondBuf->b_addr;
		}

		/* �����߼����lbn����λ��һ�μ���������еı������index */

		if (lbn < myInode::LARGE_FILE_BLOCK)
		{
			index = (lbn - myInode::SMALL_FILE_BLOCK) % myInode::ADDRESS_PER_INDEX_BLOCK;
		}
		else
		{
			index = (lbn - myInode::LARGE_FILE_BLOCK) % myInode::ADDRESS_PER_INDEX_BLOCK;
		}

		if ((phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSys.Alloc()) != NULL)
		{
			/* �����䵽���ļ������̿�ŵǼ���һ�μ���������� */
			phyBlkno = pSecondBuf->b_blkno;
			iTable[index] = phyBlkno;
			/* �������̿顢���ĺ��һ�μ�����������ӳ�д��ʽ��������� */
			bufMgr.Bdwrite(pSecondBuf);
			bufMgr.Bdwrite(pFirstBuf);
		}
		else
		{
			/* �ͷ�һ�μ��������ռ�û��� */
			bufMgr.Brelse(pFirstBuf);
		}
		/* �ҵ�Ԥ�����Ӧ�������̿�ţ������ȡԤ�������Ҫ�����һ��for����������IO�������㣬���� */
		//myInode::rablock = 0;
		if (index + 1 < myInode::ADDRESS_PER_INDEX_BLOCK)
		{
			//myInode::rablock = iTable[index + 1];
		}
		return phyBlkno;
	}
}

void myInode::IUpdate()
{
	myBuf* pBuf;
	DiskInode dInode;
	myFileSystem& filesys = myKernel::Instance().GetFileSystem();
	myBufferManager& bufMgr = myKernel::Instance().GetBufferManager();

	/* ��IUPD��IACC��־֮һ�����ã�����Ҫ������ӦDiskInode
	* Ŀ¼����������������;����Ŀ¼�ļ���IACC��IUPD��־ */
	if ((this->i_flag & (myInode::IUPD | myInode::IACC)) != 0)
	{
		if (filesys.GetFS()->s_ronly != 0)
		{
			/* ������ļ�ϵͳֻ�� */
			return;
		}

		/* ���ص�ע�ͣ��ڻ�������ҵ�������i�ڵ㣨this->i_number���Ļ����
		* ����һ�������Ļ���飬���δ����е�Bwrite()�ڽ������д�ش��̺���ͷŸû���顣
		* ���ô�Ÿ�DiskInode���ַ�����뻺���� */
		pBuf = bufMgr.Bread(myFileSystem::INODE_ZONE_START_SECTOR + this->i_number / myFileSystem::INODE_NUMBER_PER_SECTOR);

		/* ���ڴ�Inode�����е���Ϣ���Ƶ�dInode�У�Ȼ��dInode���ǻ����оɵ����Inode */
		dInode.d_mode = this->i_mode;
		dInode.d_nlink = this->i_nlink;
		dInode.d_uid = this->i_uid;
		dInode.d_gid = this->i_gid;
		dInode.d_size = this->i_size;
		for (int i = 0; i < 10; i++)
		{
			dInode.d_addr[i] = this->i_addr[i];
		}


		//ע�����ο��費��Ҫ����ʱ��
		if (this->i_flag & myInode::IACC)
		{
			/* ����������ʱ�� */
			//dInode.d_atime = time;
		}
		if (this->i_flag & myInode::IUPD)
		{
			/* ����������ʱ�� */
			//dInode.d_mtime = time;
		}

		//ע�������д�� ת��char*��ת��int*����
		/* ��pָ�򻺴����о����Inode��ƫ��λ�� */
		unsigned char* p = pBuf->b_addr + (this->i_number % myFileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
		DiskInode* pNode = &dInode;

		/* ��dInode�е������ݸ��ǻ����еľ����Inode */
		memcpy(p, pNode, sizeof(DiskInode));
		// Utility::DWordCopy((int *)pNode, (int *)p, sizeof(DiskInode) / sizeof(int));

		/* ������д�������̣��ﵽ���¾����Inode��Ŀ�� */
		bufMgr.Bwrite(pBuf);
	}
}



void myInode::ITrunc()
{
	/* ���ɴ��̸��ٻ����ȡ���һ�μ�ӡ����μ��������Ĵ��̿� */
	myBufferManager& bm = myKernel::Instance().GetBufferManager();
	/* ��ȡg_FileSystem��������ã�ִ���ͷŴ��̿�Ĳ��� */
	myFileSystem& filesys = myKernel::Instance().GetFileSystem();

	
	/* ������ַ��豸���߿��豸���˳� */
	if (this->i_mode & (myInode::IFCHR & myInode::IFBLK))
	{
		printf("in itrunc in IFCHR or IFBLK quit\n");
		return;
	}

	/* ����FILO��ʽ�ͷţ��Ծ���ʹ��SuperBlock�м�¼�Ŀ����̿��������
	*
	* Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
	* (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
	*
	* (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
	* �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
	*
	* (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
	* �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
	* (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	*/
	for (int i = 9; i >= 0; i--)		/* ��i_addr[9]��i_addr[0] */
	{
		/* ���i_addr[]�е�i��������� */
		if (this->i_addr[i] != 0)
		{
			/* �����i_addr[]�е�һ�μ�ӡ����μ�������� */
			if (i >= 6 && i <= 9)
			{
				/* �������������뻺�� */
				myBuf* pFirstBuf = bm.Bread(this->i_addr[i]);
				/* ��ȡ��������ַ */
				int* pFirst = (int *)pFirstBuf->b_addr;

				/* ÿ�ż���������¼ 512/sizeof(int) = 128�����̿�ţ�������ȫ��128�����̿� */
				for (int j = 128 - 1; j >= 0; j--)
				{
					if (pFirst[j] != 0)	/* �������������� */
					{
						/*
						* ��������μ��������i_addr[8]��i_addr[9]�
						* ��ô���ַ����¼����128��һ�μ���������ŵĴ��̿��
						*/
						if (i >= 8 && i <= 9)
						{
							myBuf* pSecondBuf = bm.Bread(pFirst[j]);
							int* pSecond = (int *)pSecondBuf->b_addr;

							for (int k = 128 - 1; k >= 0; k--)
							{
								if (pSecond[k] != 0)
								{
									/* �ͷ�ָ���Ĵ��̿� */
									filesys.Free( pSecond[k]);
								}
							}
							/* ����ʹ����ϣ��ͷ��Ա㱻��������ʹ�� */
							bm.Brelse(pSecondBuf);
						}
						filesys.Free( pFirst[j]);
					}
				}
				bm.Brelse(pFirstBuf);
			}
			/* �ͷ���������ռ�õĴ��̿� */
			filesys.Free( this->i_addr[i]);
			/* 0��ʾ����������� */
			this->i_addr[i] = 0;
		}
	}

	/* �̿��ͷ���ϣ��ļ���С���� */
	this->i_size = 0;
	/* ����IUPD��־λ����ʾ���ڴ�Inode��Ҫͬ������Ӧ���Inode */
	this->i_flag |= myInode::IUPD;
	/* ����ļ���־ ��ԭ����RWXRWXRWX����*/
	this->i_mode &= ~(myInode::ILARG & myInode::IRWXU & myInode::IRWXG & myInode::IRWXO);
	this->i_nlink = 1;
}



void myInode::Clean()
{
	/*
	* myInode::Clean()�ض�����IAlloc()������·���DiskInode��ԭ�����ݣ�
	* �����ļ���Ϣ��Clean()�����в�Ӧ�����i_dev, i_number, i_flag, i_count,
	* ���������ڴ�Inode����DiskInode�����ľ��ļ���Ϣ����Inode�๹�캯����Ҫ
	* �����ʼ��Ϊ��Чֵ��
	*/

	// this->i_flag = 0;
	this->i_mode = 0;
	//this->i_count = 0;
	this->i_nlink = 0;
	//this->i_dev = -1;
	//this->i_number = -1;
	this->i_uid = -1;
	this->i_gid = -1;
	this->i_size = 0;
	this->i_lastr = -1;
	for (int i = 0; i < 10; i++)
	{
		this->i_addr[i] = 0;
	}
}


void myInode::ICopy(myBuf *bp, int inumber)
{
	//cout << "Inode.ICopy" << endl;
	DiskInode dInode;
	DiskInode* pNode = &dInode;

	/* ��pָ�򻺴����б��Ϊinumber���Inode��ƫ��λ�� */
	unsigned char* p = bp->b_addr + (inumber % myFileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
	/* �����������Inode���ݿ�������ʱ����dInode�У���4�ֽڿ��� */
	//ע����������ͻȻ���ð����ֽڿ���������Ϊ�˺ʹ���������
	//cout << "�������е�DiskInode������һ���м���� inode�ڻ����е����" << (inumber % myFileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode) << endl;
	memcpy(pNode, p, sizeof(DiskInode));
	//Utility::DWordCopy((int *)p, (int *)pNode, sizeof(DiskInode) / sizeof(int));

	/* �����Inode����dInode����Ϣ���Ƶ��ڴ�Inode�� */
	//cout << "���DiskInode-d_mode=" << dInode.d_mode<<"  DiskInode.d_nlink=" <<dInode.d_nlink<< endl;
	this->i_mode = dInode.d_mode;
	this->i_nlink = dInode.d_nlink;
	this->i_uid = dInode.d_uid;
	this->i_gid = dInode.d_gid;
	this->i_size = dInode.d_size;
	for (int i = 0; i < 10; i++)
	{
		this->i_addr[i] = dInode.d_addr[i];
	}
	//cout << "Icopy�ɹ�����" << endl;
}



DiskInode::DiskInode()
{
	/*
	* ���DiskInodeû�й��캯�����ᷢ�����½��Ѳ���Ĵ���
	* DiskInode��Ϊ�ֲ�����ռ�ݺ���Stack Frame�е��ڴ�ռ䣬����
	* ��οռ�û�б���ȷ��ʼ�����Ծɱ�������ǰջ���ݣ����ڲ�����
	* DiskInode�����ֶζ��ᱻ���£���DiskInodeд�ص�������ʱ������
	* ����ǰջ����һͬд�أ�����д�ؽ������Ī����������ݡ�
	*/
	this->d_mode = 0;
	this->d_nlink = 0;
	this->d_uid = -1;
	this->d_gid = -1;
	this->d_size = 0;
	for (int i = 0; i < 10; i++)
	{
		this->d_addr[i] = 0;
	}
	this->d_atime = 0;
	this->d_mtime = 0;
}

DiskInode::~DiskInode()
{
	//nothing to do here
}
