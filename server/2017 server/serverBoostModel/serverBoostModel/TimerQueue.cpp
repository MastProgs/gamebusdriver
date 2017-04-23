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
	case HP_ADD: {	// 1초마다 hp 5씩 채우기

		// 이미 통신이 끊기거나 ( ai 가 죽은 녀석이면 pass )
		if (DISCONNECTED == g_clients[p->id]->get_current_connect_state()) { break; }

		int adding_hp_size = 5;

		// hp가 maxhp 이상이 아니면, 아래 실행
		if (false == (g_clients[p->id]->get_player_data()->state.hp > (g_clients[p->id]->get_player_data()->state.maxhp - 1))) {
			g_clients[p->id]->get_player_data()->state.hp += adding_hp_size;

			// 만피가 되었다면, 계속 hp 더해주는 모드 끄기
			if (g_clients[p->id]->get_player_data()->state.maxhp == g_clients[p->id]->get_player_data()->state.hp) { *g_clients[p->id]->get_hp_adding() = false; }
			add_event(p->id, 1, HP_ADD, false);
			
			sc_hp packet;
			packet.hp = g_clients[p->id]->get_player_data()->state.hp;
			packet.id = p->id;

			for (auto id : *g_clients[p->id]->get_view_list()) {
				if (DISCONNECTED == g_clients[id]->get_current_connect_state()) { continue; }
				if (true == g_clients[id]->get_player_data()->is_ai) { continue; }

				g_clients[id]->send_packet(reinterpret_cast<Packet*>(&packet));
			}

			// 컴퓨터가 아니라면, 자기한테도 한번 보내주자
			if (true == g_clients[p->id]->get_player_data()->is_ai) { break; }
			g_clients[p->id]->send_packet(reinterpret_cast<Packet*>(&packet));
		}
		break;
	}
	case DEAD_TO_ALIVE: {

		if (true == p->is_ai) {
			g_clients[p->id]->set_connect_state(CONNECTED);
			*g_clients[p->id]->get_hp_adding() = false;
			g_clients[p->id]->set_hp(g_clients[p->id]->get_maxhp());

			for (auto players : g_clients) {
				if (DISCONNECTED == players->get_current_connect_state()) { continue; }
				if (p->id == players->get_id()) { continue; }
				if (false == players->is_in_view_range(p->id)) { continue; }

				// view list 에 넣어주기
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


			//for (auto players : g_clients) {
			//	if (DISCONNECTED == players->get_current_connect_state()) { continue; }
			//	// 내 자신 아이디는 검색 안해도 됨. ai 니까..
			//	if (true == players->get_player_data()->is_ai) { continue; }
			//	players->vl_add(p->id);
			//	g_clients[p->id]->vl_add(players->get_id());
			//	players->send_packet(reinterpret_cast<Packet*>(&packet));
			//}
		}
		else {	// player 일 경우

		}

		break;
	}
	default:
		break;
	}
}