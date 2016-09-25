#pragma once
#include"stdafx.h"

void IOCP_SERVER_CLASS::IOCP_SERVER_ProcessPacket(unsigned int id, const Packet buf[])
{
	// packet[0] = packet size		> 0��° �ڸ����� ������, ��Ŷ�� ũ�Ⱑ ���߸� �Ѵ�.
	// packet[1] = type				> 1��° �ڸ����� ���� ��Ŷ�� ���� ��Ŷ���� �Ӽ��� �����ִ� ���̴�.
	// packet[...] = data			> 2��° ���ʹ� �Ӽ��� �´� ����� ó���� ���ش�.

	// buf[1] ��°�� �Ӽ����� �з��� �� �ڿ�, ���ο��� 2��° ���� �����͸� ó���ϱ� �����Ѵ�.
	switch (buf[1])
	{
	case TEST:
	{
		// ���� ��Ŷ�� �״�� �����ش�.
		IOCP_SERVER_SendPacket(id, buf);
	}
	break;
	default:
	{
		// Ŭ���̾�Ʈ�� ���� �˼� ���� �����Ͱ� ���� ���, ��ŷ ������ ���� ������ ���� ����. �ش� Ŭ���̾�Ʈ�� ���� ��ȣ�� Ÿ�� ��ȣ�� �˷��ش�.
		printf("ERROR, Unknown signal -> [ %u ] protocol num = %d\n", id, buf[1]);
		exit(-1);
	}
	break;
	}
}