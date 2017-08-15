
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
		crit	= 15,	-- ũ��Ƽ�� ����
		def		= 1,	-- ��� ����
		str		= 10,	-- �� ����
		agi		= 2,	-- ��ø ����
		intel	= 1,	-- ���� ����
		health	= 3,	-- ü�� ����
	}
}

ai_status_slime2 = {
	pos = { x = 259.83, y = 252.141 }, -- ���Ͱ� ������ �⺻ ���� X Y
	radius = 10,		-- �������� ���� ������ �ִ� �Ÿ� ���� ������
	
	howMany	= 10 + ai_status_slime.howMany,		-- �� ���� ������ ����
	aiMovSpeed = 0.3,	-- AI �ʴ� ������ �� �ִ� �ִ� �Ÿ�

	status = {
		maxHp	= 100,	-- ���� �ִ� hp
	},

	subStatus = {
		crit	= 20,
		def		= 1,
		str		= 10,
		agi		= 2,
		intel	= 1,
		health	= 3,
	}
}

ai_status_goblin = {
	pos = { x = 257.7, y = 345.5 },
	radius = 10,
	
	howMany	= 10 + ai_status_slime2.howMany,
	aiMovSpeed = 0.5,
	
	status = {
		maxHp	= 100,
	},

	subStatus = {
		crit	= 10,
		def		= 10,
		str		= 10,
		agi		= 10,
		intel	= 10,
		health	= 10,
	}
}

ai_status_goblin2 = {
	pos = { x = 259.741, y = 153.277 },
	radius = 10,
	
	howMany	= 5 + ai_status_goblin.howMany,
	aiMovSpeed = 0.5,
	
	status = {
		maxHp	= 100,
	},

	subStatus = {
		crit	= 20,
		def		= 10,
		str		= 10,
		agi		= 10,
		intel	= 10,
		health	= 10,
	}
}

ai_status_boss = {
	pos = { x = 259.4, y = 150 },
	radius = 1,
	
	howMany	= 1 + ai_status_goblin2.howMany,
	aiMovSpeed = 0.7,
	
	status = {
		maxHp	= 2000,
	},

	subStatus = {
		crit	= 30,
		def		= 15,
		str		= 20,
		agi		= 10,
		intel	= 10,
		health	= 10,
	}
}