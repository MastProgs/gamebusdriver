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

			processPacket(event_ptr);
			if (event_ptr != nullptr) { delete event_ptr; }

			time_lock.lock();
		}
		time_lock.unlock();
	}
}

void TimerQueue::add_event(const unsigned int& id, const int& sec, time_queue_event type, bool is_ai) {

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
			if (g_clients[p->id]->get_player_data()->state.maxhp == g_clients[p->id]->get_player_data()->state.hp) { *g_clients[p->id]->get_hp_adding() = false; }
			add_event(p->id, 1, HP_ADD, false);
			
			sc_hp packet;
			packet.hp = g_clients[p->id]->get_player_data()->state.hp;
			packet.id = p->id;

			for (auto players : g_clients) {
				if (DISCONNECTED == players->get_current_connect_state()) { continue; }
				//if (players->m_id == other_players->m_id) { continue; }	// �ڱ� hp �� ���ص� �ش� ��Ŷ�� �޾ƾ� �Ѵ�.
				if (true == players->get_player_data()->is_ai) { continue; }

				players->send_packet(reinterpret_cast<Packet*>(&packet));
			}
		}
		break;
	}
	case DEAD_TO_ALIVE: {

		if (true == p->is_ai) {
			g_clients[p->id]->set_connect_state(CONNECTED);
			*g_clients[p->id]->get_hp_adding() = false;
			g_clients[p->id]->set_hp(g_clients[p->id]->get_maxhp());

			sc_other_init_info packet;
			packet.playerData = *g_clients[p->id]->get_player_data();
			for (auto players : g_clients) {
				if (DISCONNECTED == players->get_current_connect_state()) { continue; }
				// �� �ڽ� ���̵�� �˻� ���ص� ��. ai �ϱ�..
				if (true == players->get_player_data()->is_ai) { continue; }
				players->send_packet(reinterpret_cast<Packet*>(&packet));
			}
		}
		else {	// player �� ���

		}

		break;
	}
	default:
		break;
	}
}