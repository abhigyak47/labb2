//
//  namesAPI.cc - a microservice demo program
//
// James Skon
// Kenyon College, 2022
//

#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <string>
#include "httplib.h"
#include <vector>


using namespace httplib;
using namespace std;

const int port = 5005;


void addMessage(string username, string message, map<string,vector<string>> &messageMap) {
	/* iterate through users adding message to each */
	string jsonMessage = "{\"user\":\""+username+"\",\"message\":\""+message+"\"}";
	for (auto userMessagePair : messageMap) {
	username = userMessagePair.first;
	messageMap[username].push_back(jsonMessage);
	}
}


void addUser(string username, string password, string email, map<string,string> &userMap) {
	string jsonMessage = "{\"user\":\""+username+"\",\"pass\":\""+password+"\",\"email\":\""+email+"\"}";
	userMap[username] = jsonMessage;
}


string getMessagesJSON(string username, map<string,vector<string>> &messageMap) {
	/* retrieve json list of messages for this user */
	bool first = true;
	string result = "{\"messages\":[";
	for (string message : messageMap[username]) {
	if (not first) result += ",";
	result += message;
	first = false;
	}
	result += "]}";
	messageMap[username].clear();
	return result;
}


int main(void) {
 Server svr;
 int nextUser=0;
 map<string,vector<string>> messageMap;
 map<string,string> userMap;
 map<string,string> userEmail;

	
 /* "/" just returnsAPI name */
 svr.Get("/", [](const Request & /*req*/, Response &res) {
 res.set_header("Access-Control-Allow-Origin","*");
 res.set_content("Chat API", "text/plain");
 });


 //microservice for registration with username, email and password
 svr.Get(R"(/chat/register/(.*)/(.*)/(.*))", [&](const Request& req, Response& res) {
	res.set_header("Access-Control-Allow-Origin","*");
 	string username = req.matches[1];
	string email = req.matches[2];
	string password = req.matches[3];
 	string result;
 	vector<string> empty;
 if (messageMap.count(username) or messageMap.count(email) or password.length() < 7){
 result = "{\"status\":\"registrationfailure\"}";
 } else {
 messageMap[username]= empty;
	userEmail[username] = email;
	addUser(username , password, email , userMap);
 result = "{\"status\":\"success\",\"user\":\"" + username + "\",\"email\":\"" + email + "\",\"pass\":\"" + password + "\"}";

 }
 res.set_content(result, "text/json");
 });
 
 
//edited for joining with username and password
 svr.Get(R"(/chat/join/(.*)/(.*))", [&](const Request& req, Response& res) {
 res.set_header("Access-Control-Allow-Origin","*");
 	string username = req.matches[1];
	string password = req.matches[2];
	string email = userEmail[username];
	string userDetails= "{\"user\":\""+username+"\",\"pass\":\""+password+"\",\"email\":\""+email+"\"}";

 string result;
 // Check if user with this name and password exists
 if (userDetails== userMap[username]){
 result = "{\"status\":\"success\",\"user\":\"" + username + "\"}";
	cout << username << " joins" << endl;
 } else {
 result = "{\"status\":\"failure\"}";
 }
 res.set_content(result, "text/json");
 });

 

 svr.Get(R"(/chat/send/(.*)/(.*))", [&](const Request& req, Response& res) {
 res.set_header("Access-Control-Allow-Origin","*");
	string username = req.matches[1];
	string message = req.matches[2];
	string result; 
 if (!messageMap.count(username)) {
 result = "{\"status\":\"baduser\"}";
	} else {
	addMessage(username,message,messageMap);
	result = "{\"status\":\"success\"}";
	}
 res.set_content(result, "text/json");
 });
 
 
 svr.Get(R"(/chat/fetch/(.*))", [&](const Request& req, Response& res) {
 string username = req.matches[1];
 res.set_header("Access-Control-Allow-Origin","*");
 string resultJSON = getMessagesJSON(username,messageMap);
 res.set_content(resultJSON, "text/json");
 });
 
//microservice for setting up user list in json	
svr.Get(R"(/chat/users)", [&](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Content-Type", "text/json");
    
    string result = "{ \"users\": [";
    for (const auto& [username, userdata] : userMap) {
        result += "\"" + username + "\", ";
    }
    if (!userMap.empty()) {
        result.erase(result.length() - 2);
    }
    result += "] }";
    
    res.set_content(result, "text/json");
});

//microservice for removing users that leave from the usermap
svr.Get(R"(/chat/users/remove/(.*))", [&](const Request& req, Response& res) {
    res.set_header("Access-Control-Allow-Origin","*");
    string username = req.matches[1];
    userMap.erase(username);
});

 
 cout << "Server listening on port " << port << endl;
 svr.listen("0.0.0.0", port);

}
