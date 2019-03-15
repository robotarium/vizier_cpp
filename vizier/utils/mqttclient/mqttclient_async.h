#ifndef VIZIER_MQTT_CLIENT_ASYNC_H
#define VIZIER_MQTT_CLIENT_ASYNC_H


#include <unordered_map>
#include <string>
#include <mosquitto.h>
#include <thread>
#include <tsqueue.h>
#include <promise.h>
#include <iostream>
#include <functional>
#include <spdlog/spdlog.h>
#include <future>


class MqttClientAsync {

private:

  std::string host;
  int port;

  std::pair<std::string, std::string> empty;
  ThreadSafeQueue<std::pair<std::string, std::string>> q;
  std::thread runner;

  ThreadSafeQueue<std::function<void()>> modifications;
  std::thread modification_thread;

  std::unordered_map<std::string, std::function<void(std::string, std::string)>> subscriptions;

  struct mosquitto* mosq;

public:

  MqttClientAsync(const std::string& host, const int port) {

    this->host = host;
    this->port = port;

    int keepalive = 20;
    bool clean_session = true;

    mosquitto_lib_init();

    mosq = mosquitto_new("overhead_tracker", clean_session, this);

    //  TODO: Actual exception handling
    if(!mosq){
      spdlog::error("Could not allocate memory");
      throw 1;
    }

    //  TODO: Actualy exception handling
    if(mosquitto_connect_bind(mosq, host.c_str(), port, keepalive, NULL)) {
      spdlog::error("Unable to connect to MQTT broker at host {0} port {1}", host, port);
      throw 2;
    }

    spdlog::info("Connected to MQTT broker at host: {0}, port: {1}", host, port);

    //  Set message callback and start the loop!
    mosquitto_message_callback_set(mosq, &MqttClientAsync::message_callback_static);
    mosquitto_connect_callback_set(mosq,&MqttClientAsync::reconnect_callback_static);
    mosquitto_loop_start(mosq);
  }

  ~MqttClientAsync(void) {

    //  Enqueue poison pill for modifications thread
    this->q.enqueue(this->empty);
    runner.join();

    //  Enqueue poison pill for modifications thread
    this->modifications.enqueue(NULL);
    this->modification_thread.join();

    //  Clean up mosquitto library
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
  }

  //  Starts all of the interal threads associated with the MQTT client
  void start() {
    runner = std::thread(&MqttClientAsync::publish_loop, this);
    modification_thread = std::thread(&MqttClientAsync::modify_loop, this);
  }

  //  Places a callback onto a particular topic such that every message to that topic is passed to callback
  //
  //  Args:
  //    topic: topic to which the MQTT client subscribes
  //    f: callback for the topic
  void subscribe_with_callback(const std::string& topic, const std::function<void(std::string, std::string)>& f) {
    auto mod = [this, topic, f]() {
      //  Can move b.c. don't care about f anymore, and we copied it to the function
      this->subscriptions[topic] = std::move(f);
      //  Struct, ?, topic string, QOS
      mosquitto_subscribe(this->mosq, NULL, topic.c_str(), 0);
    };

    this->modifications.enqueue(std::move(mod));
  }

  //  Version of subscribe that also takes a pointer to a promise to indicate when the subscription has been completed
  //
  //  Args:
  //    topic: see subscribe_with_callback
  //    f: see subscribe_with_callback 
  //    p_ptr: shared pointer to promise that is fulfilled when the subscription completes
  void subscribe_with_callback(const std::string& topic, const std::function<void(std::string, std::string)>& f, std::shared_ptr<std::promise<bool>> p_ptr) {
    auto mod = [this, topic, f, p_ptr]() {
      //  Can move b.c. don't care about f anymore, and we copied it to the function
      this->subscriptions[topic] = std::move(f);
      //  Struct, ?, topic string, QOS
      mosquitto_subscribe(this->mosq, NULL, topic.c_str(), 0);

      p_ptr->set_value(true);
    };

    this->modifications.enqueue(std::move(mod));
  }

  //  Version of subscribe that returns a queue containing incoming messages
  //
  //  Args:
  //    topic: topic to which the MQTT client subscribes
  //
  //  Returns:
  //    A pointer to queue of incoming messages 
  std::shared_ptr<ThreadSafeQueue<std::string>> subscribe(const std::string& topic) {
    std::shared_ptr<ThreadSafeQueue<std::string>> q_ptr = std::make_shared<ThreadSafeQueue<std::string>>();

    auto f = [q_ptr](std::string t, std::string msg) {
      q_ptr->enqueue(msg);
    };

    this->subscribe_with_callback(topic, std::move(f));

    return q_ptr;
  }

  //  Version of subscribe that returns a queue containing incoming messages that also takes a promise pointer to indicate when the subscription has been completed
  //
  //  Args:
  //    topic: see subscribe
  //
  //  Returns:
  //    see subscribe
  std::shared_ptr<ThreadSafeQueue<std::string>> subscribe(const std::string& topic, const std::shared_ptr<std::promise<bool>> p_ptr) {
    std::shared_ptr<ThreadSafeQueue<std::string>> q_ptr = std::make_shared<ThreadSafeQueue<std::string>>();

    auto f = [q_ptr](std::string t, std::string msg) {
      q_ptr->enqueue(msg);
    };

    this->subscribe_with_callback(topic, std::move(f), p_ptr);

    return q_ptr;
  }

  //  Unsubscribes the MQTT client from a topic.  Removes any callbacks associated with that topic.  If topic is not subscribed to, does nothing
  //  Args:
  //    topic: topic from which the MQTT client unsubscribes
  void unsubscribe(const std::string& topic) {
    auto mod = [this, topic]() {
      this->subscriptions.erase(topic.c_str());
      mosquitto_unsubscribe(mosq, NULL, topic.c_str());
    };

    this->modifications.enqueue(std::move(mod));
  }

  //  Like MqttClientAsync::unsubscribe but with a promise to indicate when the operation has been completed.
  //  Args:
  //    topic: topic from which the MQTT client unsubscribes
  void unsubscribe(const std::string& topic, std::shared_ptr<std::promise<bool>> p_ptr) {
    auto mod = [this, topic, p_ptr]() {
      this->subscriptions.erase(topic.c_str());
      mosquitto_unsubscribe(mosq, NULL, topic.c_str());

      p_ptr->set_value(true);
    };

    this->modifications.enqueue(std::move(mod));
  }

  //  Publishes a message asynchronously across the network by passing the work to a thread 
  //
  //  Args:
  //    topic: topic on which the message is published
  //    message: message to be published on the topic
  void async_publish(const std::string& topic, const std::string& message) {
    std::pair<std::string, std::string> data = {topic, message};

    if(data == this->empty) {
      spdlog::warn("Cannot publish empty message");
      return;
    }

    // We can just move this in b.c. we don't care about dt in this scope anymore
    q.enqueue(std::move(data));
  }

private:

  //  Callback for handling MQTT reconnection messages.  The subscriptions are not preserved by the server if it dies, so the client resubscribes to any existing
  //  topics on reconnect.
  //
  //  Args:
  //    mosq: pointer to Mosquitto MQTT client
  //    rc: int indicating success of connection.
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

  //  Static callback to be passed to C-implemented MQTT client.  Is called when the client connects "this" is always passed as userdata to this function when the callback
  //  is attached.
  // 
  //  Args:
  //    mosq: c-implemented MQTT client
  //    userdata: always "this" passed in
  //    rc: indicates success of connection
  static void reconnect_callback_static(struct mosquitto* mosq, void* userdata, const int rc) {
	  static_cast<MqttClientAsync*>(userdata)->reconnect_callback(mosq, rc);
  }


  //  Callback for handling incoming MQTT messages.  Is called by the C-implemented MQTT client
  //
  //  Args:
  //    mosq: Underlying c-implemented MQTT client
  //    message: message on received
  void message_callback(struct mosquitto* mosq, const struct mosquitto_message *message) {
    std::string topic((char*) message->topic);
    std::string payload((char*) message->payload);

    auto mod = [this, topic=std::move(topic), payload=std::move(payload)] {
      if(this->subscriptions.find(topic) != this->subscriptions.end()) {
        this->subscriptions[topic](std::move(topic), std::move(payload));
      }
    };

    this->modifications.enqueue(std::move(mod));
  }

  //  Static callback to be passed to C-implemented MQTT client.  Userdata is always "this"
  //
  //  Args:
  //    mosq: C-implemented MQTT client
  //    userdata: always "this"
  //    message: message received from subscribed topic
  static void message_callback_static(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
	  static_cast<MqttClientAsync*>(userdata)->message_callback(mosq, message);
  }

  //  Passed to a thread when start is called.  Stopped when destructed.
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

  //  Passed to a thread when the class is initialized.  Handles publication of messages from queue. 
  void publish_loop(void) {
    while(true) {
      auto message = q.dequeue();

      if(message == this->empty) {
        spdlog::info("Stopping publication thread");
        break;
      }

      mosquitto_publish(mosq, NULL, message.first.c_str() , message.second.length(), message.first.c_str(), 1, false);
    }
  }
};

#endif
