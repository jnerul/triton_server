这里用到了两个开源库,其实是一个
evhtp https://libevent.org/
event

TRITONSERVER_Error*
HTTPServer::Start()
{
  if (!worker_.joinable()) {
    evbase_ = event_base_new();
    htp_ = evhtp_new(evbase_, NULL);
    evhtp_enable_flag(htp_, EVHTP_FLAG_ENABLE_NODELAY);
    if (reuse_port_) {
      evhtp_enable_flag(htp_, EVHTP_FLAG_ENABLE_REUSEPORT);
    }
    evhtp_set_gencb(htp_, HTTPServer::Dispatch, this);
    evhtp_use_threads_wexit(htp_, NULL, NULL, thread_cnt_, NULL);
    if (evhtp_bind_socket(htp_, address_.c_str(), port_, 1024) != 0) {
      return TRITONSERVER_ErrorNew(
          TRITONSERVER_ERROR_UNAVAILABLE,
          (std::string("Socket '") + address_ + ":" + std::to_string(port_) +
           "' already in use ")
              .c_str());
    }

    // Set listening event for breaking event loop
    evutil_socketpair(AF_UNIX, SOCK_STREAM, 0, fds_);
    break_ev_ = event_new(evbase_, fds_[0], EV_READ, StopCallback, evbase_);
    event_add(break_ev_, NULL);
    worker_ = std::thread(event_base_loop, evbase_, 0);

    return nullptr;
  }

  return TRITONSERVER_ErrorNew(
      TRITONSERVER_ERROR_ALREADY_EXISTS, "HTTP server is already running.");
}
