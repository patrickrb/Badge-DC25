esp_err_t event_handler(void *ctx, system_event_t *event){
 if (event->event_id == SYSTEM_EVENT_SCAN_DONE) {
   //get the number of AP's found
   uint16_t apCount = event->event_info.scan_done.number;
   //setup oled for wifi menu
   oled.clearDisplay();
   currentMenuPos = 1;
   menuPosSelected = 0;
   menuNumItems = apCount;  // Allow for navigation to iterate through the correct number of positions
   menuNumber = 3;  // Each menu needs a unique ID for the cases to switch the correct function
   oled.setTextXY (0,5);
   oled.putString ("==APs==");

   wifi_ap_record_t *list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * apCount); //create list of type wifi_ap_record of the size of our results
   esp_wifi_scan_get_ap_records(&apCount, list);  //save the ap's to our list

   //loop of our list of ap's
   for (int i=0; i<apCount; i++) {
     String ssid = (char*)list[i].ssid; //convert ssid to string type
     if(ssid.length() > 13 ){ ssid.remove(13); } //trim anything over 13 chars for oled
     oled.setTextXY (i+1,2);  //move cursor
     oled.putString (ssid);  //write ssid
   }
   //cleanup
   oled.setTextXY (1,1);
   scanning = false;
   menuNavigation(menuNumber, menuNumItems);
 }
 return ESP_OK;
}
