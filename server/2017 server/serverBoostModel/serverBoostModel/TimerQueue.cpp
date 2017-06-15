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
		if (true == g_clients[p->id]->is_hp_postion) { adding_hp_size *= 2; }

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
		float x = g_clients[target_id]->get_player_data()->pos.x;
		float y = g_clients[target_id]->get_player_data()->pos.y;
		float my_x = g_clients[p->id]->get_player_data()->pos.x, my_y = g_clients[p->id]->get_player_data()->pos.y;
		float player_size = 1.5;	// ��ü �浹 ũ�� ������

		// ���� ���� �����, ������...
		if ((player_size * player_size) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {

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
				if (x > my_x) {
					my_x += movSpeed;
					//if (!(x > my_x)) { my_x = x; }
				}
				if (x < my_x) {
					my_x -= movSpeed;
					//if (!(x < my_x)) { my_x = x; }
				}
				if (y > my_y) {
					my_y += movSpeed;
					//if (!(y > my_y)) { my_y = y; }
				}
				if (y < my_y) {
					my_y -= movSpeed;
					//if (!(x < my_x)) { my_y = y; }
				}

				g_clients[p->id]->get_player_data()->pos.x = my_x;
				g_clients[p->id]->get_player_data()->pos.y = my_y;

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
	default:
		break;
	}
}