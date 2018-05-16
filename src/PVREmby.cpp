/*
 *      Copyright (C) 2011 Pulse-Eight
 *      http://www.pulse-eight.com/
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "PVREmby.h"
#include <curl/curl.h>

using namespace std;
using namespace ADDON;
using namespace rapidjson;

PVREmby::PVREmby(void)
{
  EmbyLogin();
}

PVREmby::~PVREmby(void)
{
}

size_t write_data(void *ptr, size_t size, size_t nmemb, std::string *data)
{
    size_t numBytes = size * nmemb;
    

    *data += std::string((char*)ptr, numBytes);

    return numBytes;
}


bool PVREmby::EmbyLogin(void) {
  // Read server from settings
  char buffer[1024];
  XBMC->GetSetting("server",buffer);
  m_server = buffer;
  
  // Form username/password JSON data to send to server
  Document data;
  data.SetObject();
  XBMC->GetSetting("username",buffer);
  data.AddMember("username",Value().SetString(buffer,data.GetAllocator()),data.GetAllocator());
  XBMC->GetSetting("password",buffer);
  data.AddMember("pw",Value().SetString(buffer,data.GetAllocator()),data.GetAllocator());
  // Render to string
  StringBuffer sbuffer;
  Writer<StringBuffer> writer(sbuffer);
  data.Accept(writer);

  // Initialize CURL
  CURL *curl = curl_easy_init();
  // Set URL
  curl_easy_setopt(curl, CURLOPT_URL, (m_server + "/Users/AuthenticateByName").c_str());
  // Create and set headers
  struct curl_slist *list = NULL;
  list = curl_slist_append(list, "Authorization: MediaBrowser Client=\"Kodi\", Device=\"Ubuntu\", DeviceId=\"42\", Version=\"1.0.0.0\"");
  list = curl_slist_append(list, "Content-Type: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
  // Configure user agent
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "KodiPVR");
  // Set POST data
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, sbuffer.GetString());
  // Configure function for receiving data
  std::string json;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);
  // Perform request
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  // Parse JSON result
  Document d;
  d.Parse(json.c_str());

  // Get the token and user id
  m_token = d["AccessToken"].GetString();
  m_userId = d["User"]["Id"].GetString();
  XBMC->Log(LOG_DEBUG,"Token: %s, UserId: %s",m_token.c_str(),m_userId.c_str());

}

Document PVREmby::GetURL(CStdString url) {
  // Initialize CURL
  CURL *curl = curl_easy_init();
  // Set endpoint
  curl_easy_setopt(curl, CURLOPT_URL, (m_server + url).c_str());
  // Set headers
  struct curl_slist *list = NULL;
  list = curl_slist_append(list, "Authorization: MediaBrowser Client=\"Kodi\", Device=\"Ubuntu\", DeviceId=\"42\", Version=\"1.0.0.0\"");
  list = curl_slist_append(list, ("X-MediaBrowser-Token: " + m_token).c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
  // Set user agent
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "KodiPVR");
  // Data return functions
  std::string json;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);
  // Perform request
  CURLcode res = curl_easy_perform(curl);
  // Clean-up
  curl_easy_cleanup(curl);
  // Parse received JSON data
  Document d;
  d.Parse(json.c_str());
  return d;
}

int PVREmby::GetFileContents(void* fileHandle, std::string &strContent)
{
  strContent.clear();
  if (fileHandle)
  {
    char buffer[1024];
    while (int bytesRead = XBMC->ReadFile(fileHandle, buffer, 1024))
      strContent.append(buffer, bytesRead);
    XBMC->CloseFile(fileHandle);
  }
  return strContent.length();
}

int PVREmby::GetChannelsAmount(void)
{
  // Query server
  Document data = GetURL("/LiveTV/Channels");
  // Count number of channels
  return data["Items"].Capacity();
}

PVR_ERROR PVREmby::GetChannels(ADDON_HANDLE handle, bool bRadio) {
  // Query server
  Document data = GetURL("/LiveTV/Channels");
  // Go through data
  Value& a = data["Items"];
  for (int i=0;i<a.Capacity();i++) {
    Value& channel = a[i];
    XBMC->Log(LOG_DEBUG,"Adding Channel %s",channel["Name"].GetString());      

    PVR_CHANNEL xbmcChannel;
    memset(&xbmcChannel, 0, sizeof(PVR_CHANNEL));

    xbmcChannel.iUniqueId         = atoi(channel["Number"].GetString());
    xbmcChannel.bIsRadio          = false;
    xbmcChannel.iChannelNumber    = atoi(channel["Number"].GetString());
    xbmcChannel.iSubChannelNumber = 0;
    strncpy(xbmcChannel.strChannelName, channel["Name"].GetString(), strlen(channel["Name"].GetString()));
    CStdString streamUrl = m_server + "/Videos/" + channel["Id"].GetString() + "/stream?static=true";
    strncpy(xbmcChannel.strStreamURL, streamUrl.c_str(), streamUrl.length());
    xbmcChannel.iEncryptionSystem = 0;
    CStdString strIconPath = m_server + "/Items/" + channel["Id"].GetString() + "/Images/Primary";
    strncpy(xbmcChannel.strIconPath, strIconPath.c_str(), strIconPath.length());
    xbmcChannel.bIsHidden         = false;

    PVR->TransferChannelEntry(handle, &xbmcChannel);    
    
  }
  return PVR_ERROR_NO_ERROR;
}

