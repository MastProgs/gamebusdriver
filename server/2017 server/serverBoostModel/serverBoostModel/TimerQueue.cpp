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

		// hp�� maxhp �̻��� �ƴϸ�, �Ʒ� ����
		if (false == (g_clients[p->id]->get_player_data()->state.hp > (g_clients[p->id]->get_player_data()->state.maxhp - 1))) {
			g_clients[p->id]->get_player_data()->state.hp += adding_hp_size;

			// ���ǰ� �Ǿ��ٸ�, ��� hp �����ִ� ��� ����
			if (g_clients[p->id]->get_player_data()->state.maxhp == g_clients[p->id]->get_player_data()->state.hp) { g_clients[p->id]->set_hp_adding(false); }
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

			for (auto players : g_clients) {
				if (DISCONNECTED == players->get_current_connect_state()) { continue; }
				if (p->id == players->get_id()) { continue; }
				if (false == players->is_in_view_range(p->id)) { continue; }

				// view list �� �־��ֱ�
				g_clients[p->id]->vl_add(players->get_id());
				players->vl_add(p->id);
			}

			sc_other_init_info packet;
			packet.playerData = *g_clients[p->id]->get_player_data();

			for (auto id : *g_clients[p->id]->get_view_list()) {
				if (DISCONNECTED == g_clients[id]->get_current_connect_state()) { continue; }
				if (true == g_clients[id]->get_player_data()->is_ai) { continue; }

				g_clients[id]->send_packet(reinterpret_cast<Packet*>(&packet));
			}
		}
		else {	// player �� ���

		}

		break;
	}
	case CHANGE_PLAYER_STATE: {

		if (DISCONNECTED == g_clients[p->id]->get_current_connect_state()) { break; }

		if (mov != g_clients[p->id]->get_state()) {
			g_clients[p->id]->set_state(mov);
			add_event(p->id, 1, FEVER_REDUCE, false);
		}

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

	case AI_STATE_ATT: {

		if (DISCONNECTED == g_clients[p->id]->get_current_connect_state()) { break; }
		
		unsigned int target_id = g_clients[p->id]->m_target_id;
		float x = g_clients[target_id]->get_player_data()->pos.x;
		float y = g_clients[target_id]->get_player_data()->pos.y;
		float my_x = g_clients[p->id]->get_player_data()->pos.x, my_y = g_clients[p->id]->get_player_data()->pos.y;
		float player_size = 1.5;	// ��ü �浹 ũ�� ������

		if ((player_size * player_size) >= DISTANCE_TRIANGLE(x, y, my_x, my_y)) {
			
			g_clients[target_id]->get_player_data()->state.hp -= g_clients[target_id]->get_sub_data()->str;
			int target_hp = g_clients[target_id]->get_player_data()->state.hp;
			
			sc_atk packet;
			packet.attacking_id = p->id;
			packet.under_attack_id = target_id;
			packet.hp = target_hp;

			for (auto player_id : *g_clients[p->id]->get_view_list()) {
				if (DISCONNECTED == g_clients[player_id]->get_current_connect_state()) { continue; }
				if (true == g_clients[player_id]->get_player_data()->is_ai) { continue; }

				g_clients[player_id]->send_packet(reinterpret_cast<Packet*>(&packet));
			}

			if (1 > target_hp) {
				g_clients[target_id]->set_state(mov);
				break;
			}

			//add_event(target_id, 1, HP_ADD, false);
			add_event(p->id, 1, AI_STATE_ATT, true);
		}
		else
		{
			g_clients[target_id]->set_state(mov);
		}

		break;
	}
	default:
		break;
	}
}