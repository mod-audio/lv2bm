/*
  Copyright (C) 2012 Paul Davis
  Author: David Robillard

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdlib.h>
#include <unistd.h>

#include <iostream>

#include "worker.h"

Worker::Worker(Workee* workee, uint32_t ring_size)
	: _workee(workee)
	, _requests(new RingBuffer<uint8_t>(ring_size))
	, _responses(new RingBuffer<uint8_t>(ring_size))
	, _response((uint8_t*)malloc(ring_size))
	, _sem(0)
	, _exit(false)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, 4096);
    pthread_create(&_thread, &attr, Worker::run, this);
    pthread_attr_destroy(&attr);
}

Worker::~Worker()
{
	_exit = true;
	_sem.post();
	pthread_join(_thread, NULL);
}

bool
Worker::schedule(uint32_t size, const void* data)
{
	if (_requests->write_space() < size + sizeof(size)) {
		return false;
	}
	if (_requests->write((const uint8_t*)&size, sizeof(size)) != sizeof(size)) {
		return false;
	}
	if (_requests->write((const uint8_t*)data, size) != size) {
		return false;
	}
	_sem.post();
	return true;
}

bool
Worker::respond(uint32_t size, const void* data)
{
	if (_requests->write_space() < size + sizeof(size)) {
		return false;
	}
	if (_responses->write((const uint8_t*)&size, sizeof(size)) != sizeof(size)) {
		return false;
	}
	if (_responses->write((const uint8_t*)data, size) != size) {
		return false;
	}
	return true;
}

bool
Worker::verify_message_completeness(RingBuffer<uint8_t>* rb)
{
	uint32_t read_space = rb->read_space();
	uint32_t size;
	RingBuffer<uint8_t>::rw_vector vec;
	rb->get_read_vector (&vec);
	if (vec.len[0] >= sizeof(size)) {
		memcpy (&size, vec.buf[0], sizeof (size));
	} else {
		memcpy (&size, vec.buf[0], vec.len[0]);
		memcpy (&size + vec.len[0], vec.buf[1], sizeof(size) - vec.len[0]);
	}
	if (read_space < size+sizeof(size)) {
		/* message from writer is yet incomplete. respond next cycle */
		return false;
	}
	return true;
}

void
Worker::emit_responses()
{
	uint32_t read_space = _responses->read_space();
	uint32_t size       = 0;
	while (read_space >= sizeof(size)) {
		if (!verify_message_completeness(_responses)) {
			/* message from writer is yet incomplete. respond next cycle */
			return;
		}
		/* read and send response */
		_responses->read((uint8_t*)&size, sizeof(size));
		_responses->read(_response, size);
		_workee->work_response(size, _response);
		read_space -= sizeof(size) + size;
	}
}

void*
Worker::run(void *data)
{
	Worker *worker = (Worker *) data;
	void*  buf      = NULL;
	size_t buf_size = 0;
	while (true) {
		worker->_sem.wait();
		if (worker->_exit) {
			return NULL;
		}

		uint32_t size = worker->_requests->read_space();
		if (size < sizeof(size)) {
			std::cerr << "Worker: no work-data on ring buffer" << std::endl;
			continue;
		}
		while (!worker->verify_message_completeness(worker->_requests)) {
			::usleep(2000);
			if (worker->_exit) {
				return NULL;
			}
		}
		if (worker->_requests->read((uint8_t*)&size, sizeof(size)) < sizeof(size)) {
			std::cerr << "Worker: Error reading size from request ring" << std::endl;
			continue;
		}

		if (size > buf_size) {
			buf = realloc(buf, size);
			buf_size = size;
		}

		if (worker->_requests->read((uint8_t*)buf, size) < size) {
			std::cerr << "Worker: Error reading body from request ring" << std::endl;
			continue;  // TODO: This is probably fatal
		}

		worker->_workee->work(size, buf);
	}

    free(buf);
    return NULL;
}
