
ai_differs = 3	-- ���� ��ü ����

ai_status_slime = {
	pos = { x = 134.0, y = 403.0 }, -- ���Ͱ� ������ �⺻ ���� X Y
	radius = 10,		-- �������� ���� ������ �ִ� �Ÿ� ���� ������
	
	howMany	= 10,		-- �� ���� ������ ����
	aiMovSpeed = 0.3,	-- AI �ʴ� ������ �� �ִ� �ִ� �Ÿ�

	status = {
		maxHp	= 100,	-- ���� �ִ� hp
	},

	subStatus = {
		crit	= 0,	-- ũ��Ƽ�� ����
		def		= 1,	-- ��� ����
		str		= 10,	-- �� ����
		agi		= 2,	-- ��ø ����
		intel	= 1,	-- ���� ����
		health	= 3,	-- ü�� ����
	}
}

ai_status_goblin = {
	pos = { x = 134.0, y = 403.0 },
	radius = 10,
	
	howMany	= 10 + ai_status_slime.howMany,
	aiMovSpeed = 0.5,
	
	status = {
		maxHp	= 100,
	},

	subStatus = {
		crit	= 5,
		def		= 10,
		str		= 10,
		agi		= 10,
		intel	= 10,
		health	= 10,
	}
}

ai_status_boss = {
	pos = { x = 150.0, y = 398.0 },
	radius = 1,
	
	howMany	= 1 + ai_status_goblin.howMany,
	aiMovSpeed = 0.7,
	
	status = {
		maxHp	= 1000,
	},

	subStatus = {
		crit	= 5,
		def		= 15,
		str		= 20,
		agi		= 10,
		intel	= 10,
		health	= 10,
	}
}