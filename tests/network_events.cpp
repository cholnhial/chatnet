// Run with the network.cpp and common.cpp sources and gtkmm-2.4 flags.
#include "network.h"
#include <cassert>
#include <thread>
#include <vector>

static void pump()
{
    const Glib::RefPtr<Glib::MainContext> context = Glib::MainContext::get_default();
    for(int i = 0; i < 100; ++i)
    {
        while(context->pending())
            context->iteration(false);
        usleep(1000);
    }
}

static void frame(int peer, const string& text)
{
    char data[MAX_BUFFER] = {};
    assert(text.size() <= sizeof(data));
    text.copy(data, sizeof(data));
    int length = sizeof(data);
    assert(sendall(peer, data, &length) == 0);
    pump();
}

int main()
{
    Glib::init();
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    assert(listener >= 0);
    sockaddr_in address = {};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    assert(bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
    socklen_t size = sizeof(address);
    assert(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &size) == 0);
    assert(listen(listener, 1) == 0);
    const string port = std::to_string(ntohs(address.sin_port));
    const std::thread::id owner = std::this_thread::get_id();
    std::vector<string> output;
    int errors = 0, added = 0, removed = 0;
    network net;
    net.on_signal_output().connect([&]() {
        assert(std::this_thread::get_id() == owner);
        output.push_back(net.get_buffer());
    });
    net.on_signal_new_user().connect([&]() {
        assert(std::this_thread::get_id() == owner);
        assert(net.get_buffer() == "Alice");
        ++added;
    });
    net.on_signal_remove_user().connect([&]() { ++removed; });
    net.on_signal_network_error().connect([&]() { ++errors; });
    for(int session = 0; session < 3; ++session)
    {
        assert(net.start("127.0.0.1", port.c_str(), CLIENT) == 0);
        int peer = accept(listener, NULL, NULL);
        assert(peer >= 0);
        timeval timeout = {2, 0};
        assert(setsockopt(peer, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == 0);
        net.mainloop();
        const size_t before = output.size();
        char data[MAX_BUFFER] = {};
        strcpy(data, "fragmented message");
        assert(send(peer, data, 7, 0) == 7);
        pump();
        assert(output.size() == before);
        assert(send(peer, data + 7, MAX_BUFFER - 7, 0) == MAX_BUFFER - 7);
        pump();
        assert(output.back() == "fragmented message");
        frame(peer, string(MAX_BUFFER, 'a'));
        assert(output.back() == string(MAX_BUFFER, 'a'));
        frame(peer, "add Alice");
        frame(peer, "remove Alice");
        // A chat-status response shares the same receive path as user events.
        net.input_to_server("/chat Alice");
        int length = MAX_BUFFER;
        assert(recvall(peer, data, &length) == MAX_BUFFER);
        assert(string(data) == "ischat Alice");
        frame(peer, "add Alice");
        frame(peer, "chating Tester no");
        length = MAX_BUFFER;
        assert(recvall(peer, data, &length) == MAX_BUFFER);
        assert(string(data) == "/chat Alice");
        assert(net.is_chat());
        frame(peer, "/msg Alice :hello there");
        assert(output.back() == "<Alice> hello there\n");
        close(peer);
        pump();
        assert(net.get_sockfd() == -1);
        net.reset();
    }
    assert(errors == 3 && added == 6 && removed == 3);
    close(listener);
    // Destroying a connected network removes the watch before callbacks can fire.
    return 0;
}
