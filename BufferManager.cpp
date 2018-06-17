#include "BufferManager.h"
#include "Kernel.h"
//#include"BlockDevice.h"
#include<string.h>
#include<iostream>


using namespace std;
myBufferManager::myBufferManager()
{
	//nothing to do here
}

myBufferManager::~myBufferManager()
{
	//nothing to do here
}

void myBufferManager::Initialize(char *start)
{
	//printf("in mybuffermanager ini\n");
	this->p = start;
	int i;
	myBuf* bp;

	this->bFreeList.b_forw = this->bFreeList.b_back = &(this->bFreeList);

	for (i = 0; i < NBUF; i++)
	{
		bp = &(this->m_Buf[i]);
		bp->b_addr = this->Buffer[i];
		/* ��ʼ��NODEV���� */
		//ע��NODEV��һ��������豸,��ʾ���豸
		//ע�������д�����̫ǿ��coool������
		bp->b_back = &(this->bFreeList);
		bp->b_forw = this->bFreeList.b_forw;
		this->bFreeList.b_forw->b_back = bp;
		this->bFreeList.b_forw = bp;
		/* ��ʼ�����ɶ��� */
		bp->b_flags = myBuf::B_BUSY;
		Brelse(bp);
	}
	return;
}

myBuf* myBufferManager::GetBlk( int blkno)
{
	//cout << "BufferManager.GetBlk" << endl;
	myBuf* headbp = &this->bFreeList;
	myBuf *bp;
	//ע���鿴nodev�������Ƿ��Ѿ��иÿ�Ļ���
	//��ʵ�ϱ��ο����У���������Ҫ����������У���Ϊ����������������ܴ���B_BUSY�����
	//���ǻ���Ϊ����ѭUNIXһ�������ض���������������еĴ�ͳ
	for (bp = headbp->b_forw; bp != headbp; bp = bp->b_forw)
	{
		if (bp->b_blkno != blkno)
			continue;
		bp->b_flags |= myBuf::B_BUSY;
	//	cout << "�ڻ���������ҵ���Ӧ�Ļ��棬��Ϊbusy��GetBlk���� blkno=" <<blkno<< endl;
		return bp;
	}

	//ע��������û����ͬ�Ļ����
	bp = this->bFreeList.b_forw;
	if (bp->b_flags&myBuf::B_DELWRI)
	{
	//	cout << "���䵽���ӳ�д��ǵĻ��棬��ִ��Bwrite" << endl;
		//ע�����ӳ�д��־��������ֱ��д�������첽IO�ı�־
		this->Bwrite(bp);
	}
	//ע������������������еı�־��ֻ��Ϊbusy
	bp->b_flags = myBuf::B_BUSY;

	//ע��������Ĳ����ǽ�ͷ�ڵ���β�ڵ�
	bp->b_back->b_forw = bp->b_forw;
	bp->b_forw->b_back = bp->b_back;

	bp->b_back = this->bFreeList.b_back->b_forw;
	this->bFreeList.b_back->b_forw = bp;
	bp->b_forw = &this->bFreeList;
	this->bFreeList.b_back = bp;

	bp->b_blkno = blkno;
//	cout << "�ɹ����䵽���õĻ��棬getBlk���ɹ�����" << endl;
	return bp;

} 

void myBufferManager::Brelse(myBuf* bp)
{
							/* ע�����²�����û�����B_DELWRI��B_WRITE��B_READ��B_DONE��־
							* B_DELWRI��ʾ��Ȼ���ÿ��ƿ��ͷŵ����ɶ������棬�����п��ܻ�û��Щ�������ϡ�
							* B_DONE����ָ�û����������ȷ�ط�ӳ�˴洢�ڻ�Ӧ�洢�ڴ����ϵ���Ϣ
							*/
	bp->b_flags &= ~(myBuf::B_WANTED | myBuf::B_BUSY | myBuf::B_ASYNC);
	//ע������ȡ���˶����ɻ�����еĳ�ʼ�����ƶ�������ϵͳ��ֻ��һ������


	return;
}




myBuf* myBufferManager::Bread( int blkno)
{
//	cout << "BufferManager.Bread" << endl;
	myBuf* bp;
	/* �����豸�ţ��ַ�������뻺�� */
	bp = this->GetBlk( blkno);
	/* ������豸�������ҵ����軺�棬��B_DONE�����ã��Ͳ������I/O���� */
	if (bp->b_flags & myBuf::B_DONE)
	{
	//	cout << "�ҵ����軺�棬Bread������" << endl;
		return bp;
	}
	/* û���ҵ���Ӧ���棬����I/O������� */
	bp->b_flags |= myBuf::B_READ;
	bp->b_wcount = myBufferManager::BUFFER_SIZE;
	//����ֱ�ӽ����������漴��
	//cout << "ִ�ж�blkno=" << blkno<<"  Bread���ɹ�����" << endl;
	memcpy(bp->b_addr, &p[myBufferManager::BUFFER_SIZE * blkno], myBufferManager::BUFFER_SIZE);
	

	return bp;
}



void myBufferManager::Bwrite(myBuf *bp)
{
	//cout << "BufferManager.Bwrite" << endl;
	unsigned int flags;

	flags = bp->b_flags;
	bp->b_flags &= ~(myBuf::B_READ | myBuf::B_DONE | myBuf::B_ERROR | myBuf::B_DELWRI);
	bp->b_wcount = myBufferManager::BUFFER_SIZE;		/* 512�ֽ� */

	//cout << "д�������ϵĿ��Ϊ��" << bp->b_blkno << endl;
	if ((flags & myBuf::B_ASYNC) == 0)
	{
		/* ͬ��д����Ҫ�ȴ�I/O�������� */
		memcpy(&this->p[myBufferManager::BUFFER_SIZE*bp->b_blkno], bp->b_addr, myBufferManager::BUFFER_SIZE);
		this->Brelse(bp);
	}
	else if ((flags & myBuf::B_DELWRI) == 0)
	{
		memcpy(&this->p[myBufferManager::BUFFER_SIZE*bp->b_blkno], bp->b_addr, myBufferManager::BUFFER_SIZE);
		this->Brelse(bp);
	}
	return;
}

void myBufferManager::Bdwrite(myBuf *bp)
{
	/* ����B_DONE������������ʹ�øô��̿����� */
	bp->b_flags |= (myBuf::B_DELWRI | myBuf::B_DONE);
	this->Brelse(bp);
	return;
}

void myBufferManager::Bawrite(myBuf *bp)
{
	/* ���Ϊ�첽д */
	bp->b_flags |= myBuf::B_ASYNC;
	this->Bwrite(bp);
	return;
}

void myBufferManager::ClrBuf(myBuf *bp)
{
//	cout << "BufferManager.ClrBuf" << endl;
	int* pInt = (int *)bp->b_addr;

	/* ������������������ */
	for (unsigned int i = 0; i < myBufferManager::BUFFER_SIZE / sizeof(int); i++)
	{
		pInt[i] = 0;
	}
//	cout << "���buf�е����ݣ�ClrBuf����" << endl;
	return;
}

void myBufferManager::Bflush()
{
//	cout << "Buffermanager.Bflush" << endl;
	myBuf* bp;
	//ע�������̣����ע�ⲻ��ע��
	/* ע�⣺����֮����Ҫ��������һ����֮�����¿�ʼ������
	* ��Ϊ��bwite()���뵽����������ʱ�п��жϵĲ���������
	* �ȵ�bwriteִ����ɺ�CPU�Ѵ��ڿ��ж�״̬�����Ժ�
	* �п��������ڼ���������жϣ�ʹ��bfreelist���г��ֱ仯��
	* �����������������������������¿�ʼ������ô�ܿ�����
	* ����bfreelist���е�ʱ����ִ���
	*/
	for (bp = this->bFreeList.b_forw; bp != &(this->bFreeList); bp = bp->b_forw)
	{
	//	cout << "�û����ָ����һ������" << bp->b_blkno << endl;
		/* �ҳ����ɶ����������ӳ�д�Ŀ� */
		if ((bp->b_flags & myBuf::B_DELWRI) )
		{
		//	cout << "�ҵ��ӳ�д�Ŀ�" << endl;
			//ע���������ڶ��е�β��
			bp->b_back->b_forw = bp->b_forw;
			bp->b_forw->b_back = bp->b_back;

			bp->b_back = this->bFreeList.b_back->b_forw;
			this->bFreeList.b_back->b_forw = bp;
			bp->b_forw = &this->bFreeList;
			this->bFreeList.b_back = bp;

			this->Bwrite(bp);
		}
	}
	return;
}
 

myBuf* myBufferManager::InCore( int blkno)
{
	myBuf* bp;

	myBuf*headdbp = &this->bFreeList;

	for (bp = headdbp->b_forw; bp != headdbp; bp = bp->b_forw)
	{
		if (bp->b_blkno == blkno )
			return bp;
	}
	return NULL;
}


myBuf& myBufferManager::GetBFreeList()
{
	return this->bFreeList;
}

