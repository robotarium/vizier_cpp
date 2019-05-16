#ifndef VIZIER_MQTT_CLIENT_ASYNC_H
#define VIZIER_MQTT_CLIENT_ASYNC_H


#include <unordered_map>
#include <string>
#include <mosquitto.h>
#include <thread>
#include <tsqueue.h>
#include <iostream>
#include <functional>
#include <spdlog/spdlog.h>
#include <future>
#include <optional>


using string = std::string;
template<class T> using optional = std::optional<T>;
template<class T> using shared_ptr = std::shared_ptr<T>;


class MqttClientAsync {

private:

  string host;
  int port;

  std::pair<string, string> empty;
  ThreadSafeQueue<std::pair<string, string>> q;
  std::thread runner;

  ThreadSafeQueue<std::function<void()>> modifications;
  std::thread modification_thread;

  std::unordered_map<string, std::function<void(string, string)>> subscriptions;

  struct mosquitto* mosq = NULL;

public:

  MqttClientAsync(const string& host, const int port) 
  :
  host(host),
  port(port)  
  {
    mosquitto_lib_init();
  }

  ~MqttClientAsync(void) {
    //  Enqueue poison pill for modifications thread
    this->q.enqueue(this->empty);

    if(runner.joinable()) {
      runner.join();
    }

    //  Enqueue poison pill for modifications thread
    this->modifications.enqueue(NULL);

    if(this->modification_thread.joinable()) {
      this->modification_thread.join();
    }

    //  Clean up mosquitto library
    if(mosq != NULL) {
      mosquitto_destroy(mosq);
    }

    mosquitto_lib_cleanup();
  }

  // Starts all of the interal threads.
  bool start() {
    runner = std::thread(&MqttClientAsync::publish_loop, this);
    modification_thread = std::thread(&MqttClientAsync::modify_loop, this);

    int keepalive = 20;
    bool clean_session = true;

    mosq = mosquitto_new("overhead_tracker", clean_session, this);

    if(!mosq){
      spdlog::error("Could not allocate memory");
      return false;
    }

    if(mosquitto_connect_bind(mosq, host.c_str(), port, keepalive, NULL)) {
      spdlog::error("Unable to connect to MQTT broker at host {0} port {1}", host, port);
      return false;
    }

    spdlog::info("Connected to MQTT broker at host: {0}, port: {1}", host, port);

    //  Set message callback and start the loop!
    mosquitto_message_callback_set(mosq, &MqttClientAsync::message_callback_static);
    mosquitto_connect_callback_set(mosq, &MqttClientAsync::reconnect_callback_static);
    mosquitto_loop_start(mosq);

    return true;
  }

  // Stops all of the internal threads.
  void stop() {
    //  Enqueue poison pill for modifications thread
    this->q.enqueue(this->empty);

    if(runner.joinable()) {
      runner.join();
    }

    //  Enqueue poison pill for modifications thread
    this->modifications.enqueue(NULL);

    if(this->modification_thread.joinable()) {
      this->modification_thread.join();
    }
  }

  /*  
    Version of subscribe that also takes a pointer to a promise to indicate when the subscription has been completed.  Thread safe
  
    Args:
      topic: see subscribe_with_callback
      f: see subscribe_with_callback 
      p_ptr: shared pointer to promise that is fulfilled when the subscription completes

    Returns:
      A boolean indicating if the subscription was successful
  */
  bool subscribe_with_callback(const string& topic, const std::function<void(string, string)>& f) {
    auto prom = std::make_shared<std::promise<bool>>();
    auto mod = [this, topic, f, prom]() {
      //  Can move b.c. don't care about f anymore, and we copied it to the function
      this->subscriptions[topic] = std::move(f);
      //  Struct, ?, topic string, QOS
      mosquitto_subscribe(this->mosq, NULL, topic.c_str(), 0);

      prom->set_value(true);
    };

    this->modifications.enqueue(std::move(mod));

    auto fut = prom->get_future();
    fut.wait();

    return fut.get();
  }

  /*
    Version of subscribe that returns a queue containing incoming messages.  Thread safe
 
    Args:
      topic: topic to which the MQTT client subscribes
  
    Returns:
      A pointer to queue of incoming messages 
  */
  optional<shared_ptr<ThreadSafeQueue<string>>> subscribe(const string& topic) {
    shared_ptr<ThreadSafeQueue<string>> q_ptr = std::make_shared<ThreadSafeQueue<string>>();

    auto f = [q_ptr](string t, string msg) {
      q_ptr->enqueue(msg);
    };

    bool ok = this->subscribe_with_callback(topic, std::move(f));

    if(ok) {
      return q_ptr;
    } else {
      return std::nullopt;
    }
  }

  /*
    Unsubscribes the MQTT client from a topic.  Removes any callbacks associated with that topic.  If topic is not subscribed to, does nothing.  Thread safe

    Args:
      topic: topic from which the MQTT client unsubscribes

    Returns:
      A bool indicating if the unsubscription was successful
  */ 
  bool unsubscribe(const string& topic) {
    auto prom = std::make_shared<std::promise<bool>>();
    auto mod = [this, topic, prom]() {
      this->subscriptions.erase(topic.c_str());
      mosquitto_unsubscribe(mosq, NULL, topic.c_str());

      prom->set_value(true);
    };

    this->modifications.enqueue(std::move(mod));
    
    auto fut = prom->get_future();
    fut.wait();

    return fut.get();
  }

  /*  
    Publishes a message asynchronously across the network by passing the work to a thread.  Thread safe
  
    Args:
      topic: topic on which the message is published
      message: message to be published on the topic
  */
  void async_publish(const string& topic, const string& message) {
    this->async_publish(topic, std::move(string(message)));
  }

  void async_publish(const string& topic, string&& message) {
    std::pair<string, string> data = {topic, message};

    if(data == this->empty) {
      spdlog::warn("Cannot publish empty message");
      return;
    }

    // We can just move this in b.c. we don't care about dt in this scope anymore
    q.enqueue(std::move(data));
  }

private:

  /*  
    Callback for handling MQTT reconnection messages.  The subscriptions are not preserved by the server if it dies, so the client resubscribes to any existing
    topics on reconnect.
  
    Args:
      mosq: pointer to Mosquitto MQTT client
      rc: int indicating success of connection.
  */
  void reconnect_callback(struct mosquitto* mosq, const int rc) {
    spdlog::info("Connected to broker with code {0}", rc);

    auto mod = [this] {
      for(const auto& sub : this->subscriptions) {
        spdlog::info("Resubscribing to topic {0}", sub.first);
        mosquitto_subscribe(this->mosq, NULL, sub.first.c_str(), 0);
      }
    };

    this->modifications.enqueue(std::move(mod));
  }

  /* 
    Static callback to be passed to C-implemented MQTT client.  Is called when the client connects "this" is always passed as userdata to this function when the callback
    is attached.
 
    Args:
      mosq: c-implemented MQTT client
      userdata: always "this" passed in
      rc: indicates success of connection
  */
  static void reconnect_callback_static(struct mosquitto* mosq, void* userdata, const int rc) {
	  static_cast<MqttClientAsync*>(userdata)->reconnect_callback(mosq, rc);
  }


  /* 
    Callback for handling incoming MQTT messages.  Is called by the C-implemented MQTT client
  
    Args:
      mosq: Underlying c-implemented MQTT client
      message: message on received
  */
  void message_callback(struct mosquitto* mosq, const struct mosquitto_message *message) {
    string topic((char*) message->topic);
    string payload((char*) message->payload);

    auto mod = [this, topic=std::move(topic), payload=std::move(payload)] {
      if(this->subscriptions.find(topic) != this->subscriptions.end()) {
        this->subscriptions[topic](std::move(topic), std::move(payload));
      }
    };

    this->modifications.enqueue(std::move(mod));
  }

  /*  
    Static callback to be passed to C-implemented MQTT client.  Userdata is always "this"
  
    Args:
      mosq: C-implemented MQTT client
      userdata: always "this"
      message: message received from subscribed topic
  */
  static void message_callback_static(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
	  static_cast<MqttClientAsync*>(userdata)->message_callback(mosq, message);
  }

  // Passed to a thread when start is called.  Stopped when destructed.
  void modify_loop(void) {
    while(true) {
      auto mod = modifications.dequeue();

      if(mod == NULL) {
        spdlog::info("Stopping modification thread");
        break;
      }

      mod();
    }
  }

  // Passed to a thread when the class is initialized.  Handles publication of messages from queue.  Always publishes with QoS 0
  void publish_loop(void) {
    while(true) {
      auto message = q.dequeue();

      if(message == this->empty) {
        spdlog::info("Stopping publication thread");
        break;
      }

      mosquitto_publish(mosq, NULL, message.first.c_str() , message.second.length(), message.second.c_str(), 0, false);
    }
  }
};


#endif
