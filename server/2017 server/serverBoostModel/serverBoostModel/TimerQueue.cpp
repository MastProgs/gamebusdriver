#include "stdafx.h"

void TimerQueue::TimerThread() {
	while (true) {
		Sleep(1);
		time_lock.lock();
		while (false == timer_queue.empty()) {
			if (timer_queue.top()->wakeup_time > GetTickCount()) { break; }

			event_type *event_ptr = timer_queue.top();

			timer_queue.pop();
			time_lock.unlock();

			g_io_service.post(
				[this, event_ptr]()
				{
					processPacket(event_ptr);
					if (event_ptr != nullptr) { delete event_ptr; }
				}
			);
			time_lock.lock();
		}
		time_lock.unlock();
	}
}

void TimerQueue::add_event(const unsigned int& id, const float& sec, time_queue_event type, bool is_ai) {

	event_type *ptr = new event_type;

	ptr->id = id;
	ptr->wakeup_time = GetTickCount() + (sec * 1000);
	ptr->type = type;
	ptr->is_ai = is_ai;

	time_lock.lock();
	timer_queue.push(ptr);
	time_lock.unlock();
}

void TimerQueue::processPacket(event_type *p) {
	
	switch (p->type)
	{
	case HP_ADD: {	// 1�ʸ��� hp 5�� ä���

		// �̹� ����� ����ų� ( ai �� ���� �༮�̸� pass )
		if (DISCONNECTED == g_clients[p->id]->get_current_connect_state()) { break; }

		int adding_hp_size = 5;
		if (true == g_clients[p->id]->is_hp_postion) { adding_hp_size *= 1; }

		// hp�� maxhp �̻��� �ƴϸ�, �Ʒ� ����
		if (false == (g_clients[p->id]->get_player_data()->state.hp > (g_clients[p->id]->get_player_data()->state.maxhp - 1))) {
			g_clients[p->id]->get_player_data()->state.hp += adding_hp_size;

			// ���ǰ� �Ǿ��ٸ�, ��� hp �����ִ� ��� ����
			if (g_clients[p->id]->get_player_data()->state.maxhp <= g_clients[p->id]->get_player_data()->state.hp) {
				g_clients[p->id]->get_player_data()->state.hp = g_clients[p->id]->get_player_data()->state.maxhp;
				g_clients[p->id]->set_hp_adding(false);
			}
			add_event(p->id, 1, HP_ADD, false);
			
			sc_hp packet;
			packet.hp = g_clients[p->id]->get_player_data()->state.hp;
			packet.id = p->id;

			for (auto id : *g_clients[p->id]->get_view_list()) {
				if (DISCONNECTED == g_clients[id]->get_current_connect_state()) { continue; }
				if (true == g_clients[id]->get_player_data()->is_ai) { continue; }

				g_clients[id]->send_packet(reinterpret_cast<Packet*>(&packet));
			}

			// ��ǻ�Ͱ� �ƴ϶��, �ڱ����׵� �ѹ� ��������
			if (true == g_clients[p->id]->get_player_data()->is_ai) { break; }
			g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&packet));
		}
		break;
	}
	case DEAD_TO_ALIVE: {

		if (true == p->is_ai) {
			g_clients[p->id]->set_connect_state(CONNECTED);
			g_clients[p->id]->set_hp_adding(false);
			g_clients[p->id]->set_hp(g_clients[p->id]->get_maxhp());
			g_clients[p->id]->m_target_id = none;
			g_clients[p->id]->set_state(mov);

			for (auto players : g_clients) {
				if (DISCONNECTED == players->get_current_connect_state()) { continue; }
				if (p->id == players->get_id()) { continue; }
				if (false == players->is_in_view_range(p->id)) { continue; }

				// view list �� �־��ֱ�
				g_clients[p->id]->vl_add(players->get_id());
				players->vl_add(p->id);
			}

			g_clients[p->id]->get_player_data()->pos.x = g_clients[p->id]->origin_pos.x + ((rand() % g_clients[p->id]->radius) - (g_clients[p->id]->radius / 2.0f));
			g_clients[p->id]->get_player_data()->pos.y = g_clients[p->id]->origin_pos.y + ((rand() % g_clients[p->id]->radius) - (g_clients[p->id]->radius / 2.0f));

			sc_other_init_info packet;
			packet.playerData = *g_clients[p->id]->get_player_data();
			g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet *>(&packet), p->id);

			if (true != g_clients[p->id]->ai_is_rand_mov) {
				g_clients[p->id]->m_target_id = none;
				g_clients[p->id]->set_state(mov);
				g_clients[p->id]->ai_is_rand_mov = true;
				g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
			}
		}
		else {
			// player �� ���
			if (dead != g_clients[p->id]->get_state()) { break; }

			g_clients[p->id]->set_connect_state(CONNECTED);
			g_clients[p->id]->set_state(mov);
			g_clients[p->id]->get_player_data()->pos.x = 160;
			g_clients[p->id]->get_player_data()->pos.y = 400;
			g_clients[p->id]->get_player_data()->state.mp = 10;
			g_clients[p->id]->get_player_data()->state.gauge = 0;
			g_clients[p->id]->get_player_data()->state.hp = g_clients[p->id]->get_player_data()->state.maxhp;

			sc_client_init_info init_player;
			init_player.size = sizeof(sc_client_init_info);
			init_player.type = INIT_CLIENT;
			init_player.player_info = *g_clients[p->id]->get_player_data();
			g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&init_player));
			
			// �ʱ�ȭ ���� ������ 2 - �� ������ �ٸ� �ֵ����� ������, �ٸ� �ֵ� ������ ������ ������
			sc_other_init_info my_info_to_other;
			sc_other_init_info other_info_to_me;

			my_info_to_other.playerData = *g_clients[p->id]->get_player_data();

			g_clients[p->id]->refresh_view_list(); ///

			g_clients[p->id]->vl_lock();
			for (auto id : *g_clients[p->id]->get_view_list()) {
				// �ٸ� �ֵ� ������ �����ؼ� �ְ�, ������ ���� ������...
				other_info_to_me.playerData = *(g_clients[id]->get_player_data());
				g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&other_info_to_me));

				if (true == g_clients[id]->get_player_data()->is_ai) {					
					// ai �� ��Ŷ ������ pass
					if (true != g_clients[id]->ai_is_rand_mov) {
						g_clients[id]->ai_is_rand_mov = true;
						g_time_queue.add_event(id, 0, CHANGE_AI_STATE_MOV, true);
					}

					continue;
				}
				// �ٸ� �� ������ ���� �ٸ� �ֵ����� ������ �Ǵµ�..
				g_clients[id]->send_packet(reinterpret_cast<Packet*>(&my_info_to_other));
			}
			g_clients[p->id]->vl_unlock();
		}


		break;
	}
	case CHANGE_PLAYER_STATE: {

		if (DISCONNECTED == g_clients[p->id]->get_current_connect_state()) { break; }

		if (mov != g_clients[p->id]->get_state()) {	g_clients[p->id]->set_state(mov); }

		if (true == g_clients[p->id]->get_gauge_reducing()) { break; }
		add_event(p->id, 0, FEVER_REDUCE, false);

		break;
	}

	case FEVER_REDUCE: {

		if (DISCONNECTED == g_clients[p->id]->get_current_connect_state()) { break; }

		/// ������ ������ 3�� ���� �������� �����ϵ��� ����
		if (att == g_clients[p->id]->get_state()) {
			g_clients[p->id]->set_gauge_reducing(false);
			break;
		}

		g_clients[p->id]->set_gauge_reducing(true);

		int reduce_size = 2;

		g_clients[p->id]->get_player_data()->state.gauge -= reduce_size;

		if (1 > g_clients[p->id]->get_player_data()->state.gauge) {
			g_clients[p->id]->get_player_data()->state.gauge = 0;
			g_clients[p->id]->set_gauge_reducing(false);
		}

		sc_fever packet;
		packet.gauge = g_clients[p->id]->get_player_data()->state.gauge;
		g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&packet));

		if (true == g_clients[p->id]->get_gauge_reducing()) { add_event(p->id, 1, FEVER_REDUCE, p->is_ai); }

		break;
	}

	case CHANGE_AI_STATE_ATT: {
		if (false == p->is_ai) { break; }
		if (DISCONNECTED == g_clients[p->id]->get_current_connect_state()) {
			// �� ai�� ������� ���..
			g_clients[p->id]->m_target_id = none;
			g_clients[p->id]->set_state(none);
			break;
		}

		if (none == g_clients[p->id]->m_target_id) {
			g_clients[p->id]->m_target_id = g_clients[p->id]->return_nearlest_player(RANGE_CHECK_AI_ATT);
			g_clients[p->id]->ai_is_rand_mov = true;
			if (none == g_clients[p->id]->m_target_id) {
				if (true != g_clients[p->id]->ai_is_rand_mov) {
					g_clients[p->id]->set_state(mov);
					g_clients[p->id]->ai_is_rand_mov = true;
					g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
				}
				break;
			}
		}
		g_clients[p->id]->set_state(att);

		// �Ʒ��� target id �� ã�� �����ϸ� ��
		unsigned int target_id = g_clients[p->id]->m_target_id;
		float x = g_clients[target_id]->get_player_data()->pos.x, y = g_clients[target_id]->get_player_data()->pos.y;
		float my_x = g_clients[p->id]->get_player_data()->pos.x, my_y = g_clients[p->id]->get_player_data()->pos.y;
		float player_size = 1.5;	// ��ü �浹 ũ�� ������

		// ����üũ �Լ�
		auto dir_refresh = [&]() {
			char direction_ai = 0;
			float iSize = 0.5f;
			if (((my_x + iSize) < x) && ((my_y + iSize) < y)) { direction_ai = KEYINPUT_LEFT; }
			else if (((my_x - iSize) > x) && ((my_y - iSize) > y)) { direction_ai = KEYINPUT_RIGHT; }
			else if (((my_x + iSize) < x) && ((my_y - iSize) > y)) { direction_ai = KEYINPUT_UP; }
			else if (((my_x - iSize) > x) && ((my_y + iSize) < y)) { direction_ai = KEYINPUT_DOWN; }
			else if ((my_x > x) && ((my_y - iSize) < y) && ((my_y + iSize) > y)) { direction_ai = (KEYINPUT_RIGHT | KEYINPUT_DOWN); }
			else if ((my_x < x) && ((my_y - iSize) < y) && ((my_y + iSize) > y)) { direction_ai = (KEYINPUT_LEFT | KEYINPUT_UP); }
			else if ((my_y > y) && ((my_x - iSize) < x) && ((my_x + iSize) > x)) { direction_ai = (KEYINPUT_RIGHT | KEYINPUT_UP); }
			else if ((my_y < y) && ((my_x - iSize) < x) && ((my_x + iSize) > x)) { direction_ai = (KEYINPUT_LEFT | KEYINPUT_DOWN); }
			else { cout << "AI Direction ERROR\n"; }
			
			return direction_ai;
		};

		// [ ���� ]
		if (p->id == MAX_AI_BOSS - 1) {

			g_clients[p->id]->get_player_data()->dir = dir_refresh();
			cout << "BOSS ATTCKED\n";

			int boss_skill_cnt = BOSS_ATT_06 - BOSS_ATT;
			srand((unsigned)time(NULL));
			player_size = 2.3;
			{
				switch ((rand() % boss_skill_cnt) + BOSS_ATT)
				{
				case BOSS_ATT_06:
				case BOSS_ATT_02:
				case BOSS_ATT_01: {
					if ((player_size * player_size) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
						g_clients[target_id]->get_player_data()->state.hp -= (g_clients[p->id]->get_sub_data()->str - g_clients[target_id]->get_sub_data()->def);
						int target_hp = g_clients[target_id]->get_player_data()->state.hp;

						sc_boss_atk b_atk;
						b_atk.att_type = BOSS_ATT_01;

						sc_atk damage_packet;
						damage_packet.attacking_id = p->id;
						damage_packet.under_attack_id = target_id;
						damage_packet.hp = target_hp;

						sc_dir dir_packet_refresh;
						dir_packet_refresh.dir = g_clients[p->id]->get_player_data()->dir;
						dir_packet_refresh.id = p->id;
						
						g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&b_atk), p->id);
						g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet_refresh), p->id);

						g_clients[target_id]->vl_lock();
						for (auto player_id : *g_clients[target_id]->get_view_list()) {
							if (DISCONNECTED == g_clients[player_id]->get_current_connect_state()) { continue; }
							if (true == g_clients[player_id]->get_player_data()->is_ai) { continue; }

							g_clients[player_id]->send_packet(reinterpret_cast<Packet*>(&damage_packet));
						}
						g_clients[target_id]->vl_unlock();
						g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&damage_packet));
						
						// ���� ���� �÷��̾ �׾��ٸ�..?
						if (1 > target_hp) {
							g_clients[target_id]->set_state(dead);

							sc_disconnect dis_p;
							dis_p.id = target_id;
							g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&dis_p));
							g_clients[target_id]->set_connect_state(DISCONNECTED);

							g_clients[target_id]->vl_lock();
							for (auto player_view_ids : *g_clients[target_id]->get_view_list()) {
								// dead lock ������ continue;
								//if (player_view_ids == p->id) { deleting_id = target_id; continue; }
								g_clients[player_view_ids]->get_view_list()->erase(target_id);

								sc_disconnect dis_p_to_me;
								dis_p_to_me.id = player_view_ids;
								g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&dis_p_to_me));

								if (true == g_clients[player_view_ids]->get_player_data()->is_ai) { continue; }
								g_clients[player_view_ids]->send_packet(reinterpret_cast<Packet*>(&dis_p));
							}
							g_clients[target_id]->get_view_list()->clear();
							g_clients[target_id]->vl_unlock();
							g_time_queue.add_event(target_id, 5, DEAD_TO_ALIVE, false);

							if (true != g_clients[p->id]->ai_is_rand_mov) {
								g_clients[p->id]->ai_is_rand_mov = true;
								g_clients[p->id]->m_target_id = none;
								g_clients[p->id]->set_state(mov);
								g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
							}

							break;
						}

						g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_ATT, true);
					}
					else {
						// ���� ���� ���̶��, ���󰡾� ��... ����� ��û
						if ((VIEW_RANGE * VIEW_RANGE) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
							float movSpeed = g_clients[p->id]->ai_mov_speed * 2;
							if (x > my_x) { my_x += movSpeed; }
							if (x < my_x) { my_x -= movSpeed; }
							if (y > my_y) { my_y += movSpeed; }
							if (y < my_y) { my_y -= movSpeed; }

							g_clients[p->id]->get_player_data()->pos.x = my_x;
							g_clients[p->id]->get_player_data()->pos.y = my_y;

							sc_dir dir_packet;
							dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
							dir_packet.id = p->id;
							g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);

							sc_move pac;
							pac.id = p->id;
							pac.pos = g_clients[p->id]->get_player_data()->pos;
							g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&pac), p->id);

							g_time_queue.add_event(p->id, 1, CHANGE_AI_STATE_ATT, true);
						}
						else {
							// �ƿ� �þ� ���� ���̶��, �ʱ�ȭ �ʿ�
							if (true == g_clients[p->id]->ai_is_rand_mov) {
								g_clients[p->id]->m_target_id = none;
								g_clients[p->id]->set_state(mov);
								g_clients[p->id]->ai_is_rand_mov = true;
								g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
							}
						}
					}

					break;
				}
				//case BOSS_ATT_02: {
				//	if (SQUARED(player_size) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
				//
				//		//vector<unsigned int> attcked_id;
				//
				//		// ���⿡ ���� ���� ��ġ ����
				//		char my_dir = g_clients[p->id]->get_player_data()->dir;
				//		position atk_spot = g_clients[p->id]->get_player_data()->pos;
				//		float atk_spot_move_size = 1.5;
				//		if ((my_dir & KEYINPUT_RIGHT) == (KEYINPUT_RIGHT)) { atk_spot.x -= atk_spot_move_size; atk_spot.y -= atk_spot_move_size; }
				//		if ((my_dir & KEYINPUT_LEFT) == (KEYINPUT_LEFT)) { atk_spot.x += atk_spot_move_size; atk_spot.y += atk_spot_move_size; }
				//		if ((my_dir & KEYINPUT_UP) == (KEYINPUT_UP)) { atk_spot.x += atk_spot_move_size; atk_spot.y -= atk_spot_move_size; }
				//		if ((my_dir & KEYINPUT_DOWN) == (KEYINPUT_DOWN)) { atk_spot.x -= atk_spot_move_size; atk_spot.y += atk_spot_move_size; }
				//
				//		// �浹 ������ �ִ� ���� �˻�
				//		for (auto players : g_clients) {
				//			if (DISCONNECTED == players->get_current_connect_state()) { continue; }
				//			if (p->id >= players->get_id()) { continue; }
				//
				//			float atked_x = players->get_player_data()->pos.x, atked_y = players->get_player_data()->pos.y;
				//			if (SQUARED(player_size) >= DISTANCE_TRIANGLE(atk_spot.x, atk_spot.y, atked_x, atked_y)) {
				//				//attcked_id.emplace_back(players->get_id());
				//				players->get_player_data()->state.hp -= (g_clients[p->id]->get_sub_data()->str - players->get_sub_data()->def);
				//				int target_hp = players->get_player_data()->state.hp;
				//
				//				sc_atk damage_packet;
				//				damage_packet.attacking_id = p->id;
				//				damage_packet.under_attack_id = players->get_id();
				//				damage_packet.hp = target_hp;
				//
				//				players->vl_lock();
				//				for (auto player_id : *players->get_view_list()) {
				//					if (DISCONNECTED == g_clients[player_id]->get_current_connect_state()) { continue; }
				//					if (true == g_clients[player_id]->get_player_data()->is_ai) { continue; }
				//
				//					g_clients[player_id]->send_packet(reinterpret_cast<Packet*>(&damage_packet));
				//				}
				//				players->vl_unlock();
				//				players->send_packet(reinterpret_cast<Packet*>(&damage_packet));
				//
				//				// ���� ���� �÷��̾ �׾��ٸ�..?
				//				if (1 > target_hp) {
				//					players->set_state(dead);
				//
				//					sc_disconnect dis_p;
				//					dis_p.id = players->get_id();
				//					players->send_packet(reinterpret_cast<Packet*>(&dis_p));
				//					players->set_connect_state(DISCONNECTED);
				//
				//					players->vl_lock();
				//					for (auto player_view_ids : *players->get_view_list()) {
				//						g_clients[player_view_ids]->get_view_list()->erase(players->get_id());
				//
				//						sc_disconnect dis_p_to_me;
				//						dis_p_to_me.id = player_view_ids;
				//						players->send_packet(reinterpret_cast<Packet*>(&dis_p_to_me));
				//
				//						if (true == g_clients[player_view_ids]->get_player_data()->is_ai) { continue; }
				//						g_clients[player_view_ids]->send_packet(reinterpret_cast<Packet*>(&dis_p));
				//					}
				//					players->get_view_list()->clear();
				//					players->vl_unlock();
				//					g_time_queue.add_event(players->get_id(), 5, DEAD_TO_ALIVE, false);
				//
				//					if (true != g_clients[p->id]->ai_is_rand_mov) {
				//						g_clients[p->id]->ai_is_rand_mov = true;
				//						g_clients[p->id]->m_target_id = none;
				//						g_clients[p->id]->set_state(mov);
				//						g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
				//					}
				//
				//					break;
				//				}
				//			}
				//		}
				//		
				//		sc_boss_atk b_atk;
				//		b_atk.att_type = BOSS_ATT_02;
				//
				//		sc_dir dir_packet_refresh;
				//		dir_packet_refresh.dir = g_clients[p->id]->get_player_data()->dir;
				//		dir_packet_refresh.id = p->id;
				//
				//		g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&b_atk), p->id);
				//		g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet_refresh), p->id);
				//								
				//		g_time_queue.add_event(p->id, 2.5f, CHANGE_AI_STATE_ATT, true);
				//	}
				//	else {
				//		// ���� ���� ���̶��, ���󰡾� ��... ����� ��û
				//		if (SQUARED(VIEW_RANGE) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
				//			float movSpeed = g_clients[p->id]->ai_mov_speed * 2;
				//			if (x > my_x) { my_x += movSpeed; }
				//			if (x < my_x) { my_x -= movSpeed; }
				//			if (y > my_y) { my_y += movSpeed; }
				//			if (y < my_y) { my_y -= movSpeed; }
				//
				//			g_clients[p->id]->get_player_data()->pos.x = my_x;
				//			g_clients[p->id]->get_player_data()->pos.y = my_y;
				//
				//			sc_dir dir_packet;
				//			dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
				//			dir_packet.id = p->id;
				//			g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);
				//
				//			sc_move pac;
				//			pac.id = p->id;
				//			pac.pos = g_clients[p->id]->get_player_data()->pos;
				//			g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&pac), p->id);
				//
				//			g_time_queue.add_event(p->id, 1, CHANGE_AI_STATE_ATT, true);
				//		}
				//		else {
				//			// �ƿ� �þ� ���� ���̶��, �ʱ�ȭ �ʿ�
				//			if (true == g_clients[p->id]->ai_is_rand_mov) {
				//				g_clients[p->id]->m_target_id = none;
				//				g_clients[p->id]->set_state(mov);
				//				g_clients[p->id]->ai_is_rand_mov = true;
				//				g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
				//			}
				//		}
				//	}
				//	break;
				//}
				case BOSS_ATT_03: {
					g_time_queue.add_event(p->id, 1.3f, TIMER_ATT, true);

					sc_dir dir_packet;
					dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
					dir_packet.id = p->id;
					g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);

					sc_boss_atk b_atk;
					b_atk.att_type = BOSS_ATT_03;
					g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&b_atk), p->id);
					break;
				}
				case BOSS_ATT_04: {
					g_time_queue.add_event(p->id, 2.0f, TIMER_ATT_BOSS_4, true);
					
					sc_boss_atk b_atk;
					b_atk.att_type = BOSS_ATT_04;
					g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&b_atk), p->id);
					break;
				}
				case BOSS_ATT_05: {
					if ((player_size * player_size) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
						g_clients[target_id]->get_player_data()->state.hp -= (g_clients[p->id]->get_sub_data()->str - g_clients[target_id]->get_sub_data()->def);
						int target_hp = g_clients[target_id]->get_player_data()->state.hp;

						sc_boss_atk b_atk;
						b_atk.att_type = BOSS_ATT_05;

						sc_atk damage_packet;
						damage_packet.attacking_id = p->id;
						damage_packet.under_attack_id = target_id;
						damage_packet.hp = target_hp;

						sc_dir dir_packet_refresh;
						dir_packet_refresh.dir = g_clients[p->id]->get_player_data()->dir;
						dir_packet_refresh.id = p->id;

						g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&b_atk), p->id);
						g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet_refresh), p->id);

						g_clients[target_id]->vl_lock();
						for (auto player_id : *g_clients[target_id]->get_view_list()) {
							if (DISCONNECTED == g_clients[player_id]->get_current_connect_state()) { continue; }
							if (true == g_clients[player_id]->get_player_data()->is_ai) { continue; }

							g_clients[player_id]->send_packet(reinterpret_cast<Packet*>(&damage_packet));
						}
						g_clients[target_id]->vl_unlock();
						g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&damage_packet));

						// ���� ���� �÷��̾ �׾��ٸ�..?
						if (1 > target_hp) {
							g_clients[target_id]->set_state(dead);

							sc_disconnect dis_p;
							dis_p.id = target_id;
							g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&dis_p));
							g_clients[target_id]->set_connect_state(DISCONNECTED);

							g_clients[target_id]->vl_lock();
							for (auto player_view_ids : *g_clients[target_id]->get_view_list()) {
								// dead lock ������ continue;
								//if (player_view_ids == p->id) { deleting_id = target_id; continue; }
								g_clients[player_view_ids]->get_view_list()->erase(target_id);

								sc_disconnect dis_p_to_me;
								dis_p_to_me.id = player_view_ids;
								g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&dis_p_to_me));

								if (true == g_clients[player_view_ids]->get_player_data()->is_ai) { continue; }
								g_clients[player_view_ids]->send_packet(reinterpret_cast<Packet*>(&dis_p));
							}
							g_clients[target_id]->get_view_list()->clear();
							g_clients[target_id]->vl_unlock();
							g_time_queue.add_event(target_id, 5, DEAD_TO_ALIVE, false);

							if (true != g_clients[p->id]->ai_is_rand_mov) {
								g_clients[p->id]->ai_is_rand_mov = true;
								g_clients[p->id]->m_target_id = none;
								g_clients[p->id]->set_state(mov);
								g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
							}

							break;
						}

						g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_ATT, true);
					}
					else {
						// ���� ���� ���̶��, ���󰡾� ��... ����� ��û
						if ((VIEW_RANGE * VIEW_RANGE) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
							float movSpeed = g_clients[p->id]->ai_mov_speed * 2;
							if (x > my_x) { my_x += movSpeed; }
							if (x < my_x) { my_x -= movSpeed; }
							if (y > my_y) { my_y += movSpeed; }
							if (y < my_y) { my_y -= movSpeed; }

							g_clients[p->id]->get_player_data()->pos.x = my_x;
							g_clients[p->id]->get_player_data()->pos.y = my_y;

							sc_dir dir_packet;
							dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
							dir_packet.id = p->id;
							g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);

							sc_move pac;
							pac.id = p->id;
							pac.pos = g_clients[p->id]->get_player_data()->pos;
							g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&pac), p->id);

							g_time_queue.add_event(p->id, 1, CHANGE_AI_STATE_ATT, true);
						}
						else {
							// �ƿ� �þ� ���� ���̶��, �ʱ�ȭ �ʿ�
							if (true == g_clients[p->id]->ai_is_rand_mov) {
								g_clients[p->id]->m_target_id = none;
								g_clients[p->id]->set_state(mov);
								g_clients[p->id]->ai_is_rand_mov = true;
								g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
							}
						}
					}

					break;
				}
				default:
					break;
				}
			}

			break;
		}
		// [ ��� ]
		else if ((p->id < MAX_AI_GOBLIN) && (p->id > MAX_AI_SLIME)) {

			if (((VIEW_RANGE - 5) * (VIEW_RANGE - 5)) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
				sc_dir dir_packet;
				dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
				dir_packet.id = p->id;
				g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);

				g_clients[target_id]->get_player_data()->state.hp -= (g_clients[p->id]->get_sub_data()->str - g_clients[target_id]->get_sub_data()->def);
				int target_hp = g_clients[target_id]->get_player_data()->state.hp;

				sc_atk packet;
				packet.attacking_id = p->id;
				packet.under_attack_id = target_id;
				packet.hp = target_hp;

				g_clients[target_id]->vl_lock();
				for (auto player_id : *g_clients[target_id]->get_view_list()) {
					if (DISCONNECTED == g_clients[player_id]->get_current_connect_state()) { continue; }
					if (true == g_clients[player_id]->get_player_data()->is_ai) { continue; }

					g_clients[player_id]->send_packet(reinterpret_cast<Packet*>(&packet));
				}
				g_clients[target_id]->vl_unlock();
				g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&packet));

				// ���� ���� �÷��̾ �׾��ٸ�..?
				if (1 > target_hp) {
					g_clients[target_id]->set_state(dead);

					sc_disconnect dis_p;
					dis_p.id = target_id;
					g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&dis_p));
					g_clients[target_id]->set_connect_state(DISCONNECTED);

					g_clients[target_id]->vl_lock();
					for (auto player_view_ids : *g_clients[target_id]->get_view_list()) {
						// dead lock ������ continue;
						//if (player_view_ids == p->id) { deleting_id = target_id; continue; }
						g_clients[player_view_ids]->get_view_list()->erase(target_id);

						sc_disconnect dis_p_to_me;
						dis_p_to_me.id = player_view_ids;
						g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&dis_p_to_me));

						if (true == g_clients[player_view_ids]->get_player_data()->is_ai) { continue; }
						g_clients[player_view_ids]->send_packet(reinterpret_cast<Packet*>(&dis_p));
					}
					g_clients[target_id]->get_view_list()->clear();
					g_clients[target_id]->vl_unlock();
					g_time_queue.add_event(target_id, 5, DEAD_TO_ALIVE, false);

					if (true != g_clients[p->id]->ai_is_rand_mov) {
						g_clients[p->id]->ai_is_rand_mov = true;
						g_clients[p->id]->m_target_id = none;
						g_clients[p->id]->set_state(mov);
						g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
					}

					break;
				}

				g_time_queue.add_event(p->id, 1, CHANGE_AI_STATE_ATT, true);
			}
			// ���ʿ� ���Ÿ� �����̴�, �þ� ���� ���̶�� �� ���·� �ʱ�ȭ
			else if (true == g_clients[p->id]->ai_is_rand_mov) {
				g_clients[p->id]->m_target_id = none;
				g_clients[p->id]->set_state(mov);
				g_clients[p->id]->ai_is_rand_mov = true;
				g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
			}
			break;
		}

		// [ ������ ] ���� ���� �����, ������...
		if ((player_size * player_size) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {

			sc_dir dir_packet;
			dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
			dir_packet.id = p->id;
			g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);

			g_clients[target_id]->get_player_data()->state.hp -= (g_clients[p->id]->get_sub_data()->str - g_clients[target_id]->get_sub_data()->def);
			int target_hp = g_clients[target_id]->get_player_data()->state.hp;

			sc_atk packet;
			packet.attacking_id = p->id;
			packet.under_attack_id = target_id;
			packet.hp = target_hp;
			
			g_clients[target_id]->vl_lock();
			for (auto player_id : *g_clients[target_id]->get_view_list()) {
				if (DISCONNECTED == g_clients[player_id]->get_current_connect_state()) { continue; }
				if (true == g_clients[player_id]->get_player_data()->is_ai) { continue; }

				g_clients[player_id]->send_packet(reinterpret_cast<Packet*>(&packet));
			}
			g_clients[target_id]->vl_unlock();
			g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&packet));

			// ���� ���� �÷��̾ �׾��ٸ�..?
			if (1 > target_hp) {
				g_clients[target_id]->set_state(dead);
				
				sc_disconnect dis_p;
				dis_p.id = target_id;
				g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&dis_p));
				g_clients[target_id]->set_connect_state(DISCONNECTED);

				g_clients[target_id]->vl_lock();
				for (auto player_view_ids : *g_clients[target_id]->get_view_list()) {
					// dead lock ������ continue;
					//if (player_view_ids == p->id) { deleting_id = target_id; continue; }
					g_clients[player_view_ids]->get_view_list()->erase(target_id);
					
					sc_disconnect dis_p_to_me;
					dis_p_to_me.id = player_view_ids;
					g_clients[target_id]->send_packet(reinterpret_cast<Packet*>(&dis_p_to_me));

					if (true == g_clients[player_view_ids]->get_player_data()->is_ai) { continue; }
					g_clients[player_view_ids]->send_packet(reinterpret_cast<Packet*>(&dis_p));
				}
				g_clients[target_id]->get_view_list()->clear();
				g_clients[target_id]->vl_unlock();
				g_time_queue.add_event(target_id, 5, DEAD_TO_ALIVE, false);

				if (true != g_clients[p->id]->ai_is_rand_mov) {
					g_clients[p->id]->ai_is_rand_mov = true;
					g_clients[p->id]->m_target_id = none;
					g_clients[p->id]->set_state(mov);
					g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
				}

				break;
			}

			g_time_queue.add_event(p->id, 1, CHANGE_AI_STATE_ATT, true);
		}
		else {
			// ���� ���� ���̶��, ���󰡾� ��... ����� ��û
			if ((VIEW_RANGE * VIEW_RANGE) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
				float movSpeed = g_clients[p->id]->ai_mov_speed * 2;
				if (x > my_x) { my_x += movSpeed; }
				if (x < my_x) { my_x -= movSpeed; }
				if (y > my_y) { my_y += movSpeed; }
				if (y < my_y) { my_y -= movSpeed; }

				g_clients[p->id]->get_player_data()->pos.x = my_x;
				g_clients[p->id]->get_player_data()->pos.y = my_y;

				sc_dir dir_packet;
				dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
				dir_packet.id = p->id;
				g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);

				sc_move pac;
				pac.id = p->id;
				pac.pos = g_clients[p->id]->get_player_data()->pos;
				g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&pac), p->id);

				g_time_queue.add_event(p->id, 1, CHANGE_AI_STATE_ATT, true);
			}
			else {
				// �ƿ� �þ� ���� ���̶��, �ʱ�ȭ �ʿ�
				if (true == g_clients[p->id]->ai_is_rand_mov) {
					g_clients[p->id]->m_target_id = none;
					g_clients[p->id]->set_state(mov);
					g_clients[p->id]->ai_is_rand_mov = true;
					g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
				}
			}			
		}

		// �÷��̾ ����ߴٸ�.. ���� 
		/// 1. �ֺ� Ÿ�� �˻�
		/// 2. Ÿ�� ������ ���� none

		break;
	}
							  
	case CHANGE_AI_STATE_MOV: {

		if (false == p->is_ai) { break; }
		if (DISCONNECTED == g_clients[p->id]->get_current_connect_state()) {
			// �� ai�� ������� ���..
			g_clients[p->id]->m_target_id = none;
			g_clients[p->id]->set_state(none);
			g_clients[p->id]->ai_is_rand_mov = false;
			break;
		}

		if (none != g_clients[p->id]->return_nearlest_player(VIEW_RANGE)) {
			g_clients[p->id]->ai_is_rand_mov = true;
			g_clients[p->id]->m_target_id = g_clients[p->id]->ai_rand_mov();

			if (none != g_clients[p->id]->m_target_id) {
				if (att == g_clients[p->id]->get_state()) { break; }
				g_time_queue.add_event(p->id, 0, CHANGE_AI_STATE_ATT, true);
				break;
			}

			g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
		}
		else {
			g_clients[p->id]->m_target_id = none;
			g_clients[p->id]->set_state(none);
			g_clients[p->id]->ai_is_rand_mov = false;
			break;
		}

		break;
	}

	case POSTION: {

		g_clients[p->id]->is_hp_postion = false;

		break;
	}

	case TIMER_ATT_BOSS_4: {
		if (p->id == MAX_AI_BOSS - 1) {
			// �Ʒ��� target id �� ã�� �����ϸ� ��
			unsigned int target_id = g_clients[p->id]->m_target_id;
			float x = g_clients[target_id]->get_player_data()->pos.x, y = g_clients[target_id]->get_player_data()->pos.y;
			float my_x = g_clients[p->id]->get_player_data()->pos.x, my_y = g_clients[p->id]->get_player_data()->pos.y;
			float player_size = 1.5;	// ��ü �浹 ũ�� ������
			
			// [ ���� ]
			if (p->id == MAX_AI_BOSS - 1) {
				if (SQUARED(3) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {

					// �浹 ������ �ִ� ���� �˻�
					for (auto players : g_clients) {
						if (DISCONNECTED == players->get_current_connect_state()) { continue; }
						if (p->id >= players->get_id()) { continue; }

						float atked_x = players->get_player_data()->pos.x, atked_y = players->get_player_data()->pos.y;
						if (SQUARED(3) >= DISTANCE_TRIANGLE(atked_x, atked_y, my_x, my_y)) {
							players->get_player_data()->state.hp -= (g_clients[p->id]->get_sub_data()->str - players->get_sub_data()->def);
							int target_hp = players->get_player_data()->state.hp;

							sc_atk damage_packet;
							damage_packet.attacking_id = p->id;
							damage_packet.under_attack_id = players->get_id();
							damage_packet.hp = target_hp;

							players->vl_lock();
							for (auto player_id : *players->get_view_list()) {
								if (DISCONNECTED == g_clients[player_id]->get_current_connect_state()) { continue; }
								if (true == g_clients[player_id]->get_player_data()->is_ai) { continue; }

								g_clients[player_id]->send_packet(reinterpret_cast<Packet*>(&damage_packet));
							}
							players->vl_unlock();
							players->send_packet(reinterpret_cast<Packet*>(&damage_packet));

							// ���� ���� �÷��̾ �׾��ٸ�..?
							if (1 > target_hp) {
								players->set_state(dead);

								sc_disconnect dis_p;
								dis_p.id = players->get_id();
								players->send_packet(reinterpret_cast<Packet*>(&dis_p));
								players->set_connect_state(DISCONNECTED);

								players->vl_lock();
								for (auto player_view_ids : *players->get_view_list()) {
									g_clients[player_view_ids]->get_view_list()->erase(players->get_id());

									sc_disconnect dis_p_to_me;
									dis_p_to_me.id = player_view_ids;
									players->send_packet(reinterpret_cast<Packet*>(&dis_p_to_me));

									if (true == g_clients[player_view_ids]->get_player_data()->is_ai) { continue; }
									g_clients[player_view_ids]->send_packet(reinterpret_cast<Packet*>(&dis_p));
								}
								players->get_view_list()->clear();
								players->vl_unlock();
								g_time_queue.add_event(players->get_id(), 5, DEAD_TO_ALIVE, false);

								if (true != g_clients[p->id]->ai_is_rand_mov) {
									g_clients[p->id]->ai_is_rand_mov = true;
									g_clients[p->id]->m_target_id = none;
									g_clients[p->id]->set_state(mov);
									g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
								}

								break;
							}
						}
					}

					sc_dir dir_packet_refresh;
					dir_packet_refresh.dir = g_clients[p->id]->get_player_data()->dir;
					dir_packet_refresh.id = p->id;
					g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet_refresh), p->id);

					g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_ATT, true);
				}
				else {
					// ���� ���� ���̶��, ���󰡾� ��... ����� ��û
					if (SQUARED(VIEW_RANGE) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
						float movSpeed = g_clients[p->id]->ai_mov_speed * 2;
						if (x > my_x) { my_x += movSpeed; }
						if (x < my_x) { my_x -= movSpeed; }
						if (y > my_y) { my_y += movSpeed; }
						if (y < my_y) { my_y -= movSpeed; }

						g_clients[p->id]->get_player_data()->pos.x = my_x;
						g_clients[p->id]->get_player_data()->pos.y = my_y;

						// ����üũ �Լ�
						auto dir_refresh = [&]() {
							char direction_ai = 0;
							float iSize = 0.5f;
							if (((my_x + iSize) < x) && ((my_y + iSize) < y)) { direction_ai = KEYINPUT_LEFT; }
							else if (((my_x - iSize) > x) && ((my_y - iSize) > y)) { direction_ai = KEYINPUT_RIGHT; }
							else if (((my_x + iSize) < x) && ((my_y - iSize) > y)) { direction_ai = KEYINPUT_UP; }
							else if (((my_x - iSize) > x) && ((my_y + iSize) < y)) { direction_ai = KEYINPUT_DOWN; }
							else if ((my_x > x) && ((my_y - iSize) < y) && ((my_y + iSize) > y)) { direction_ai = (KEYINPUT_RIGHT | KEYINPUT_DOWN); }
							else if ((my_x < x) && ((my_y - iSize) < y) && ((my_y + iSize) > y)) { direction_ai = (KEYINPUT_LEFT | KEYINPUT_UP); }
							else if ((my_y > y) && ((my_x - iSize) < x) && ((my_x + iSize) > x)) { direction_ai = (KEYINPUT_RIGHT | KEYINPUT_UP); }
							else if ((my_y < y) && ((my_x - iSize) < x) && ((my_x + iSize) > x)) { direction_ai = (KEYINPUT_LEFT | KEYINPUT_DOWN); }
							else { cout << "AI Direction ERROR\n"; }

							return direction_ai;
						};

						sc_dir dir_packet;
						dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
						dir_packet.id = p->id;
						g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);

						sc_move pac;
						pac.id = p->id;
						pac.pos = g_clients[p->id]->get_player_data()->pos;
						g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&pac), p->id);

						g_time_queue.add_event(p->id, 1, CHANGE_AI_STATE_ATT, true);
					}
					else {
						// �ƿ� �þ� ���� ���̶��, �ʱ�ȭ �ʿ�
						if (true == g_clients[p->id]->ai_is_rand_mov) {
							g_clients[p->id]->m_target_id = none;
							g_clients[p->id]->set_state(mov);
							g_clients[p->id]->ai_is_rand_mov = true;
							g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
						}
					}
				}
			}

		}
		else { cout << "TIMER ERROR\n"; }

		break;
	}
	case TIMER_ATT: {

		if (true == p->is_ai) {
			// �Ʒ��� target id �� ã�� �����ϸ� ��
			unsigned int target_id = g_clients[p->id]->m_target_id;
			float x = g_clients[target_id]->get_player_data()->pos.x, y = g_clients[target_id]->get_player_data()->pos.y;
			float my_x = g_clients[p->id]->get_player_data()->pos.x, my_y = g_clients[p->id]->get_player_data()->pos.y;
			float player_size = 1.5;	// ��ü �浹 ũ�� ������
			
			// [ ���� ]
			if (p->id == MAX_AI_BOSS - 1) {
				auto is_in_boss_skill_range = [&](position &other) {

					// ���⿡ ���� ���� ��ġ ����
					char my_dir = g_clients[p->id]->get_player_data()->dir;
					position spot1, spot2, spot3, spot4;
					position atk_spot = g_clients[p->id]->get_player_data()->pos;
					float atk_spot_move_size = 2;
					// ���� �簢�� ������ ���ϱ�
					spot1 = spot2 = atk_spot;
					{
						if ((my_dir & KEYINPUT_MASKUD) == 0) {
							spot1.x += atk_spot_move_size;
							spot1.y -= atk_spot_move_size;
							spot2.x -= atk_spot_move_size;
							spot2.y += atk_spot_move_size;
							if ((my_dir & KEYINPUT_RIGHT) == (KEYINPUT_RIGHT)) {
								spot3.x = spot1.x - (atk_spot_move_size * 7);
								spot3.y = spot1.y - (atk_spot_move_size * 7);
								spot4.x = spot2.x - (atk_spot_move_size * 7);
								spot4.y = spot2.y - (atk_spot_move_size * 7);
							}
							else if ((my_dir & KEYINPUT_LEFT) == (KEYINPUT_LEFT)) {
								spot3.x = spot1.x + (atk_spot_move_size * 7);
								spot3.y = spot1.y + (atk_spot_move_size * 7);
								spot4.x = spot2.x + (atk_spot_move_size * 7);
								spot4.y = spot2.y + (atk_spot_move_size * 7);
							}
						}
						else {
							if ((my_dir & KEYINPUT_MASKLR) == 0) {
								spot1.x += atk_spot_move_size;
								spot1.y += atk_spot_move_size;
								spot2.x -= atk_spot_move_size;
								spot2.y -= atk_spot_move_size;
								if ((my_dir & KEYINPUT_UP) == (KEYINPUT_UP)) {
									spot3.x = spot1.x + (atk_spot_move_size * 7);
									spot3.y = spot1.y - (atk_spot_move_size * 7);
									spot4.x = spot2.x + (atk_spot_move_size * 7);
									spot4.y = spot2.y - (atk_spot_move_size * 7);
								}
								else if ((my_dir & KEYINPUT_DOWN) == (KEYINPUT_DOWN)) {
									spot3.x = spot1.x - (atk_spot_move_size * 7);
									spot3.y = spot1.y + (atk_spot_move_size * 7);
									spot4.x = spot2.x - (atk_spot_move_size * 7);
									spot4.y = spot2.y + (atk_spot_move_size * 7);
								}
							}
							else {
								/// ���Ʒ� ���¿��� �ٸ� �¿� ���� ������ ( �밢�� )
								if ((my_dir & (KEYINPUT_RIGHT | KEYINPUT_DOWN)) == (KEYINPUT_RIGHT | KEYINPUT_DOWN)) {
									spot1.y -= atk_spot_move_size;
									spot2.y += atk_spot_move_size;
									spot3 = spot1;
									spot3.x -= (atk_spot_move_size * 7);
									spot4 = spot2;
									spot4.x -= (atk_spot_move_size * 7);
								}
								else if ((my_dir & (KEYINPUT_RIGHT | KEYINPUT_UP)) == (KEYINPUT_RIGHT | KEYINPUT_UP)) {
									spot1.x += atk_spot_move_size;
									spot2.x -= atk_spot_move_size;
									spot3 = spot1;
									spot3.y -= (atk_spot_move_size * 7);
									spot4 = spot2;
									spot4.y -= (atk_spot_move_size * 7);
								}
								else if ((my_dir & (KEYINPUT_LEFT | KEYINPUT_DOWN)) == (KEYINPUT_LEFT | KEYINPUT_DOWN)) {
									spot1.x += atk_spot_move_size;
									spot2.x -= atk_spot_move_size;
									spot3 = spot1;
									spot3.y += (atk_spot_move_size * 7);
									spot4 = spot2;
									spot4.y += (atk_spot_move_size * 7);
								}
								else if ((my_dir & (KEYINPUT_LEFT | KEYINPUT_UP)) == (KEYINPUT_LEFT | KEYINPUT_UP)) {
									spot1.y -= atk_spot_move_size;
									spot2.y += atk_spot_move_size;
									spot3 = spot1;
									spot3.x += (atk_spot_move_size * 7);
									spot4 = spot2;
									spot4.x += (atk_spot_move_size * 7);
								}
							}
						}
					}
					vector<position> atk_range;
					atk_range.reserve(4);
					atk_range.emplace_back(spot1);
					atk_range.emplace_back(spot2);
					atk_range.emplace_back(spot3);
					atk_range.emplace_back(spot4);

					// �浹 ���� üũ�ϱ�
					int crosses = 0;
					for (int i = 0; i < atk_range.size(); i++) {
						int j = (i + 1) % atk_range.size();
						//�� B�� ���� (p[i], p[j])�� y��ǥ ���̿� ����
						if ((atk_range[i].y > other.y) != (atk_range[j].y > other.y)) {
							//atX�� �� B�� ������ ���򼱰� ���� (p[i], p[j])�� ����
							float atX = (atk_range[j].x - atk_range[i].x) * (other.y - atk_range[i].y) / (atk_range[j].y - atk_range[i].y) + atk_range[i].x;
							//atX�� ������ ���������� ������ ������ ������ ������ ������Ų��.
							if (other.x < atX)
								crosses++;
						}
					}
					return (crosses & 1);
				};

				if (SQUARED(6.5) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {

					// �浹 ������ �ִ� ���� �˻�
					for (auto players : g_clients) {
						if (DISCONNECTED == players->get_current_connect_state()) { continue; }
						if (p->id >= players->get_id()) { continue; }

						float atked_x = players->get_player_data()->pos.x, atked_y = players->get_player_data()->pos.y;
						if (true == is_in_boss_skill_range(players->get_player_data()->pos)) {
							players->get_player_data()->state.hp -= (g_clients[p->id]->get_sub_data()->str - players->get_sub_data()->def);
							int target_hp = players->get_player_data()->state.hp;

							sc_atk damage_packet;
							damage_packet.attacking_id = p->id;
							damage_packet.under_attack_id = players->get_id();
							damage_packet.hp = target_hp;

							players->vl_lock();
							for (auto player_id : *players->get_view_list()) {
								if (DISCONNECTED == g_clients[player_id]->get_current_connect_state()) { continue; }
								if (true == g_clients[player_id]->get_player_data()->is_ai) { continue; }

								g_clients[player_id]->send_packet(reinterpret_cast<Packet*>(&damage_packet));
							}
							players->vl_unlock();
							players->send_packet(reinterpret_cast<Packet*>(&damage_packet));

							// ���� ���� �÷��̾ �׾��ٸ�..?
							if (1 > target_hp) {
								players->set_state(dead);

								sc_disconnect dis_p;
								dis_p.id = players->get_id();
								players->send_packet(reinterpret_cast<Packet*>(&dis_p));
								players->set_connect_state(DISCONNECTED);

								players->vl_lock();
								for (auto player_view_ids : *players->get_view_list()) {
									g_clients[player_view_ids]->get_view_list()->erase(players->get_id());

									sc_disconnect dis_p_to_me;
									dis_p_to_me.id = player_view_ids;
									players->send_packet(reinterpret_cast<Packet*>(&dis_p_to_me));

									if (true == g_clients[player_view_ids]->get_player_data()->is_ai) { continue; }
									g_clients[player_view_ids]->send_packet(reinterpret_cast<Packet*>(&dis_p));
								}
								players->get_view_list()->clear();
								players->vl_unlock();
								g_time_queue.add_event(players->get_id(), 5, DEAD_TO_ALIVE, false);

								if (true != g_clients[p->id]->ai_is_rand_mov) {
									g_clients[p->id]->ai_is_rand_mov = true;
									g_clients[p->id]->m_target_id = none;
									g_clients[p->id]->set_state(mov);
									g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
								}

								break;
							}
						}
					}

					sc_dir dir_packet_refresh;
					dir_packet_refresh.dir = g_clients[p->id]->get_player_data()->dir;
					dir_packet_refresh.id = p->id;
					g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet_refresh), p->id);

					g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_ATT, true);
				}
				else {
					// ���� ���� ���̶��, ���󰡾� ��... ����� ��û
					if (SQUARED(VIEW_RANGE) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
						float movSpeed = g_clients[p->id]->ai_mov_speed * 2;
						if (x > my_x) { my_x += movSpeed; }
						if (x < my_x) { my_x -= movSpeed; }
						if (y > my_y) { my_y += movSpeed; }
						if (y < my_y) { my_y -= movSpeed; }

						g_clients[p->id]->get_player_data()->pos.x = my_x;
						g_clients[p->id]->get_player_data()->pos.y = my_y;

						// ����üũ �Լ�
						auto dir_refresh = [&]() {
							char direction_ai = 0;
							float iSize = 0.5f;
							if (((my_x + iSize) < x) && ((my_y + iSize) < y)) { direction_ai = KEYINPUT_LEFT; }
							else if (((my_x - iSize) > x) && ((my_y - iSize) > y)) { direction_ai = KEYINPUT_RIGHT; }
							else if (((my_x + iSize) < x) && ((my_y - iSize) > y)) { direction_ai = KEYINPUT_UP; }
							else if (((my_x - iSize) > x) && ((my_y + iSize) < y)) { direction_ai = KEYINPUT_DOWN; }
							else if ((my_x > x) && ((my_y - iSize) < y) && ((my_y + iSize) > y)) { direction_ai = (KEYINPUT_RIGHT | KEYINPUT_DOWN); }
							else if ((my_x < x) && ((my_y - iSize) < y) && ((my_y + iSize) > y)) { direction_ai = (KEYINPUT_LEFT | KEYINPUT_UP); }
							else if ((my_y > y) && ((my_x - iSize) < x) && ((my_x + iSize) > x)) { direction_ai = (KEYINPUT_RIGHT | KEYINPUT_UP); }
							else if ((my_y < y) && ((my_x - iSize) < x) && ((my_x + iSize) > x)) { direction_ai = (KEYINPUT_LEFT | KEYINPUT_DOWN); }
							else { cout << "AI Direction ERROR\n"; }

							return direction_ai;
						};

						sc_dir dir_packet;
						dir_packet.dir = g_clients[p->id]->get_player_data()->dir = dir_refresh();
						dir_packet.id = p->id;
						g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&dir_packet), p->id);

						sc_move pac;
						pac.id = p->id;
						pac.pos = g_clients[p->id]->get_player_data()->pos;
						g_clients[p->id]->send_packet_other_players_in_view_range(reinterpret_cast<Packet*>(&pac), p->id);

						g_time_queue.add_event(p->id, 1, CHANGE_AI_STATE_ATT, true);
					}
					else {
						// �ƿ� �þ� ���� ���̶��, �ʱ�ȭ �ʿ�
						if (true == g_clients[p->id]->ai_is_rand_mov) {
							g_clients[p->id]->m_target_id = none;
							g_clients[p->id]->set_state(mov);
							g_clients[p->id]->ai_is_rand_mov = true;
							g_time_queue.add_event(p->id, 3, CHANGE_AI_STATE_MOV, true);
						}
					}
				}
			}

			break;
		}
		else {
			// �÷��̾� �ð�����
			// �浹üũ �˻��ϰ� �� �ڿ�..
			float att_x = 0.3, att_y = 0.3;		// �׽�Ʈ�� Ŭ�� ���� ��ġ�� ������
			float my_x = g_clients[p->id]->get_player_data()->pos.x, my_y = g_clients[p->id]->get_player_data()->pos.y;
			float player_size = 1.35;	// ��ü �浹 ũ�� ������
			char *dir = &g_clients[p->id]->get_player_data()->dir;
			bool is_gauge_on = false;
			unsigned int deleting_id = 0;

			if ((*dir & KEYINPUT_RIGHT) == (KEYINPUT_RIGHT)) { my_x -= att_x; my_y -= att_y; }
			if ((*dir & KEYINPUT_LEFT) == (KEYINPUT_LEFT)) { my_x += att_x; my_y += att_y; }
			if ((*dir & KEYINPUT_UP) == (KEYINPUT_UP)) { my_x += att_x; my_y -= att_y; }
			if ((*dir & KEYINPUT_DOWN) == (KEYINPUT_DOWN)) { my_x -= att_x; my_y += att_y; }

			g_clients[p->id]->vl_lock();
			for (auto id : *g_clients[p->id]->get_view_list()) {
				if (DISCONNECTED == g_clients[id]->get_current_connect_state()) { continue; }
				//if (m_id == g_clients[id]->m_id) { continue; }
				//if (true == players->get_player_data()->is_ai) { continue; }

				float x = g_clients[id]->get_player_data()->pos.x;
				float y = g_clients[id]->get_player_data()->pos.y;

				// ������ �ߴµ� ��밡 �¾Ҵٰ� ������ �ȴٸ�...================================================================
				if ((player_size * player_size) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {

					// �� ���� ���� ���� --------------
					g_clients[p->id]->set_state(att);

					// ���� ���� �޺� �� ��ų�� ���� ������ �������ֱ�
					int addingDamage = 0;

					// �ϴ� ��� ü�� ����
					std::random_device rd;
					std::mt19937_64 mt(rd());
					std::uniform_int_distribution<int> dist(30, g_clients[p->id]->get_sub_data()->critical + 30);
					g_clients[id]->get_player_data()->state.hp -= ((g_clients[p->id]->get_sub_data()->str + addingDamage + dist(mt)) - g_clients[id]->get_sub_data()->def);
					is_gauge_on = true; // �߿� �������� ������ üũ �� �÷�����

					sc_atk p_atk;
					p_atk.attacking_id = p->id;		// ������ id
					p_atk.under_attack_id = id;		// �´� ���� id
					p_atk.hp = g_clients[id]->get_player_data()->state.hp;	// ���� ���� hp
					p_atk.comboState = ATK_COMBO_ETC;

					// hp �� 0 �� �Ǹ� ���ó���� �Ѵ�. -> ������ Ŭ���̾�Ʈ���� hp �� 0 �� �༮�� ������
					if (0 >= g_clients[id]->get_player_data()->state.hp) {

						// ���� �ְ� ai �� �׳� ���� ��� ���̱�
						if (MAX_AI_NUM > g_clients[id]->get_id()) {
							g_clients[id]->set_connect_state(DISCONNECTED);
							g_clients[id]->ai_is_rand_mov = false;
							g_clients[id]->set_state(none);

							sc_disconnect dis_p;
							dis_p.id = id;

							g_clients[p->id]->send_packet_other_players(reinterpret_cast<Packet *>(&dis_p), id);
							g_time_queue.add_event(g_clients[id]->get_player_data()->id, 10, DEAD_TO_ALIVE, true);	// bot ����

																												// ���� �༮���� ����ġ�� �ֵ��� ����.
																												// ���� �������̰�, quest ��ġ�� 10 ���� ��°� ���϶��
							if ((MAX_AI_SLIME > id) && (MAX_AI_SLIME > g_clients[p->id]->get_sub_data()->quest)) {
								g_clients[p->id]->get_sub_data()->quest += 1;

								sc_chat chat;
								chat.id = p->id;
								wsprintfW(reinterpret_cast<wchar_t*>(chat.msg), L"������ %d ���� ����", g_clients[p->id]->get_sub_data()->quest);
								if (MAX_AI_SLIME == g_clients[p->id]->get_sub_data()->quest) { wsprintfW(reinterpret_cast<wchar_t*>(chat.msg), L"������ ����Ʈ �Ϸ�"); }
								g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&chat));

								sc_quest q;
								q.quest = g_clients[p->id]->get_sub_data()->quest;
								g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&q));
							}
							else if ((MAX_AI_GOBLIN > id) && (MAX_AI_SLIME <= id) && (MAX_AI_GOBLIN > g_clients[p->id]->get_sub_data()->quest) && (MAX_AI_SLIME <= g_clients[p->id]->get_sub_data()->quest)) {
								g_clients[p->id]->get_sub_data()->quest += 1;

								sc_chat chat;
								chat.id = p->id;
								wsprintfW(reinterpret_cast<wchar_t*>(chat.msg), L"��� %d ���� ����", g_clients[p->id]->get_sub_data()->quest - MAX_AI_SLIME);
								if (MAX_AI_GOBLIN == g_clients[p->id]->get_sub_data()->quest) { wsprintfW(reinterpret_cast<wchar_t*>(chat.msg), L"��� ����Ʈ �Ϸ�");; }
								g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&chat));

								sc_quest q;
								q.quest = g_clients[p->id]->get_sub_data()->quest;
								g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&q));
							}
						}
						else {
							// ���� �ְ� player �� ���..

							sc_disconnect dis_p;
							dis_p.id = id;

							g_clients[id]->send_packet(reinterpret_cast<Packet*>(&dis_p));
							g_clients[id]->set_state(dead);

							for (auto player_view_ids : *g_clients[id]->get_view_list()) {
								// dead lock ������ continue;
								if (player_view_ids == p->id) { deleting_id = id; continue; }
								g_clients[player_view_ids]->vl_remove(id);

								if (true == g_clients[player_view_ids]->get_player_data()->is_ai) { continue; }
								g_clients[player_view_ids]->send_packet(reinterpret_cast<Packet*>(&dis_p));
							}

							g_clients[id]->vl_clear();
							g_time_queue.add_event(g_clients[id]->get_player_data()->id, 5, DEAD_TO_ALIVE, false);
						}
					}

					g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&p_atk));
					if (true == g_clients[id]->get_player_data()->is_ai) { continue; }
					g_clients[id]->send_packet(reinterpret_cast<Packet*>(&p_atk));
				}
			}
			g_clients[p->id]->vl_unlock();
			if (0 < deleting_id) { g_clients[p->id]->vl_remove(deleting_id); }
		}

		break;
	}
	default:
		break;
	}
}