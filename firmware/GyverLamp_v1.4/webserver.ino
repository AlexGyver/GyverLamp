void webserver(){
  
  http.onNotFound(routeNotFound);

  /** главная */
  http.on("/", routeHome); 
  /** прием конфигурации */
  http.on("/setconfig", routeSetConfig); 
  /** получить текущие настройки/конфигурацию */
  http.on("/getconfig", routeGetConfig); 
  
  /** страница настройка таймера вкл/выкл */
//  http.on("/timer", routeTimer); 
  /** прием конфигурации таймера вкл/выкл */
//  http.on("/settimerconfig", routeGetTimerConfig); 
  /** получить текущие настройки/конфигурацию таймера вкл/выкл */
//  http.on("/gettimerconfig", routeSetTimerConfig); 
  
  /** страница настройка будильника */
  http.on("/alarm", routeAlarm); 
  /** прием конфигурации будильника */
  http.on("/setalarmconfig", routeSetAlarmConfig); 
  /** получить текущие настройки/конфигурацию будильника */
  http.on("/getalarmconfig", routeGetAlarmConfig); 
  
  http.begin();
  
}

/**
 * шаблон/отправщик страницы
 */
void responseHtml(String out, String title = "AlexGyver Lamp", int code = 200){ 
  String html;
  
  html = "<html>";
    html += "<head>";
      html += "<title>" + title + "</title>";
      html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\" />";
      html += "<link rel=\"stylesheet\" href=\"https://demos.jquerymobile.com/1.4.5/css/themes/default/jquery.mobile-1.4.5.min.css\">";
      html += "<link rel=\"stylesheet\" href=\"http://fonts.googleapis.com/css?family=Open+Sans:300,400,700\">";
      html += "<script src=\"https://demos.jquerymobile.com/1.4.5/js/jquery.js\"></script>";
      html += "<script src=\"https://demos.jquerymobile.com/1.4.5/js/jquery.mobile-1.4.5.min.js\"></script>";
    html += "</head>";
    html += "<body>";
      html += "<div data-role='page'>";
        html += "<div data-role='header' class='jqm-header'><h2><a class='ui-link' data-ajax='false' href='/'><img style='width: 100%' src='//i0.wp.com/alexgyver.ru/wp-content/uploads/2018/07/site_mob-1.png'></a></h2></div><!-- /header -->";
        html += "<div role='main' class='ui-content jqm-content' style='padding-bottom: 100px;'>";
        html += "";
        html += out;
        html += "";
        html += "</div>";
        html += "<div data-role='footer' data-theme='b' style='position: fixed;width: 100%;bottom: 0;z-index: 1;'>";
            html += "<div data-role='navbar' data-iconpos='bottom'>";
                html += "<ul>";
                    html += "<li><a href='/' data-ajax='false' data-icon='gear'>Основные настройки</a></li>"; // сдлеать активной class='ui-btn-active'
                    html += "<li><a href='/alarm' data-ajax='false' data-icon='clock'>Будильник</a></li>";
                    html += "<!--<li><a href='/timer' data-ajax='false' data-icon='power'>Расписание</a></li>-->";
                html += "</ul>";
            html += "</div>"; // .navbar
        html += "</div>"; // .footer
      html += "</div>"; // .page

      // js функция отправки/получения данных формы
      html += "    <script type=\"text/javascript\">\n";
      html += "    function syncConfig(getconfig = '/getconfig', setconfig = '/setconfig'){\n";
      html += "        $.ajax({url: getconfig, dataType:'json', success: init});\n";
      html += "        function init(config){\n";
      html += "            /**\n";
      html += "             * костыль/фича\n";
      html += "             * лень было искать как нормально установить параметы в виджеты\n";
      html += "             * пока не установятся параметры из ESP, отправка не будет осуществляться\n";
      html += "             * @type {boolean}\n";
      html += "             */\n";
      html += "            window.changeReaction = false;\n";
      html += "                let timeouts = {};\n";
      html += "                $('select, input').on('change',(v) => {\n";
      html += "                    /** если данные не пришли, ничего не отправляем/сохраненяем */\n";
      html += "                    if(window.changeReaction === false) return;\n";
      html += "                    let that = $(v.currentTarget), name = that.attr('name'), outData = {};\n";
      html += "                    /** если в очереди на отправку/сохранение есть такой параметр, то снимим предыдущее значение */\n";
      html += "                    if(timeouts[name] != undefined)\n";
      html += "                        clearTimeout(timeouts[name]);\n";
      html += "                    /**\n";
      html += "                     * установим измененный параметр в очередь на отправку/сохранение в ESP\n";
      html += "                     * @type {number}\n";
      html += "                     */\n";
      html += "                    timeouts[name] = setTimeout(() => {\n";
      html += "                        window.changeReaction = false;\n";
      html += "                        outData[name] = that.val();\n";
      html += "                        $.ajax({\n";
      html += "                            url:setconfig,\n";
      html += "                            data: outData,\n";
      html += "                            dataType: 'json',\n";
      html += "                            success: (json) => {\n";
      html += "                                if(json[name] = outData[name])\n";
      html += "                                    window.changeReaction = true;\n";
      html += "                                else\n";
      html += "                                    alert('Не удалось сохранить настройки.');\n";
      html += "                                setConfig(json);\n";
      html += "                            },\n";
      html += "                            error: (event) => alert(`Не удалось сохранить настройки.\nПроизошла ошибка \"${event.status} ${event.statusText}\".`)\n";
      html += "                        });\n";
      html += "                    }, 500);\n";
      html += "                });\n";
      html += "                setConfig(config);\n";
      html += "                /** установим актуальные параметры из ESP */\n";
      html += "                function setConfig(config){\n";
      html += "                  window.changeReaction = false;\n";
      html += "                  for (let key in config){\n";
      html += "                      let that = $(`[name=${key}]`);\n";
      html += "                      that.val(config[key]);\n";
      html += "                      that.trigger('change');\n";
      html += "                  }\n";
      html += "                  /**\n";
      html += "                   * разрешаем вносить изменеия в конфигу\n";
      html += "                   * @type {boolean}\n";
      html += "                   */\n";
      html += "                  window.changeReaction = true;\n";
      html += "                }\n";
      html += "        }\n";
      html += "    }\n";
      html += "    </script>\n";
      
    html += "</body>";
  html += "</html>";
  
  http.sendHeader("Cache-Control","max-age=0, private, must-revalidate");
  http.send(code, "text/html; charset=utf-8", html); 
}

/**
 * исключение/вывод ошибки о не найденном пути
 */
void routeNotFound() {
  String out;
  
  out = "Путь не найден";
  out += "<br />URI: ";
  out += http.uri();
  out += "<br />Method: ";
  out += (http.method() == HTTP_GET) ? "GET" : "POST";
  out += "<br />Arguments: ";
  out += http.args();
  out += "<br /><pre>";
  for (uint8_t i = 0; i < http.args(); i++) {
    out += " " + http.argName(i) + ": " + http.arg(i) + "<br />";
  }
  out += "</pre><hr /><a class='ui-link' data-ajax='false' href=\"/\">Перейти на главную</a>";
  responseHtml(out, "Ошибка 404", 404);
}

/**
 * отправка текущей конфигурации 
 * + отправка JSON(обязательно должен завершаться запятой)
 */
void routeGetConfig(){
  String out;

  out += "{";
  out += "\"currentMode\":\"" + String(currentMode) + "\",";
  out += "\"brightness\":\"" + String(modes[currentMode].brightness) + "\",";
  out += "\"speed\":\"" + String(modes[currentMode].speed) + "\",";
  out += "\"scale\":\"" + String(modes[currentMode].scale) + "\",";
  out += "\"on\":\"" + String(ONflag) + "\"";
  out += "}";
  
  http.send(200, "text/json", out);
}

/**
 * изменение/применение новой конфигурации
 */
void routeSetConfig(){
  String out;
  
  if(http.hasArg("on")){
    
    ONflag = (http.arg("on").toInt() > 0) ? true : false;
    changePower();
    
  }
  
  if(http.hasArg("currentMode")){
    
    String value;

    value = http.arg("currentMode");
    
    currentMode =  value.toInt();
           
    if (currentMode >= MODE_AMOUNT || currentMode < 0) currentMode = 0;
    
    manualOff = true;
    dawnFlag = false;
    loadingFlag = true;
    saveEEPROM();
    loadingFlag = true;
    FastLED.clear();
    FastLED.setBrightness(modes[currentMode].brightness);
    delay(1);
    changePower();
  }
  
  if(http.hasArg("scale")){
    modes[currentMode].scale = http.arg("scale").toInt();
    loadingFlag = true;
    settChanged = true;
    eepromTimer = millis();
  }
  
  if(http.hasArg("brightness")){
    modes[currentMode].brightness = http.arg("brightness").toInt();
    FastLED.setBrightness(modes[currentMode].brightness);
    settChanged = true;
    eepromTimer = millis();
  }
  
  if(http.hasArg("speed")){
    modes[currentMode].speed = http.arg("speed").toInt();
    loadingFlag = true;
    settChanged = true;
    eepromTimer = millis();
  }
  
  /** в знак завершения операции отправим текущую конфигурацию */
  routeGetConfig();
  
}

void routeAlarm(){
  String out, days[] = {"пн","вт","ср","чт","пт","сб","вс"};
  out = "<form>";
    for (byte i = 0; i < 7; i++) {
      out += "<div class='ui-field-contain'>";  
          out += "<label for='day_" + String(i) + "'>"+days[i]+"</label>";
          out += "<select name='day_" + String(i) + "' id='day_" + String(i) + "' data-role='slider' data-mini='true'>";
              out += "<option value='0'></option>";
              out += "<option value='1'></option>";
          out += "</select>";
      out += "</div>";
      out += "<div class='ui-field-contain'><label for='time_" + String(i) + "'>время</label><input name='time_" + String(i) + "' id='time_" + String(i) + "' type='time' value='00:00' /></div>";
    }
    
    out += "<div class='ui-field-contain'>";
      out += "<label for='dawnMode'>Рассвета за:</label>";
      out += "<select name='dawnMode' id='dawnMode'>";
      for(byte i = 0; i <= sizeof(dawnOffsets) - 1; i++){
       out += "<option value='" + String(i) + "'>" + String(dawnOffsets[i]) + "</option>"; 
      }
      out += "</select>";
    out += "</div>";
  out += "</form>";
  
  out += "<script type='text/javascript'>$(document).ready(()=>{syncConfig('/getalarmconfig','/setalarmconfig');});</script>";
  
  responseHtml(out);
}

void routeSetAlarmConfig(){
  
  for (byte i = 0; i < 7; i++) {
    
    if(http.hasArg("day_"+String(i))){
      alarm[i].state = (http.arg("day_" + String(i)).toInt() > 0);
      saveAlarm(i);
    }
    if(http.hasArg("time_"+String(i))){
      
      alarm[i].time = http.arg("time_" + String(i)).substring(0,2).toInt() * 60 + http.arg("time_" + String(i)).substring(3,5).toInt();
      saveAlarm(i);
    }
  }
  
  if(http.hasArg("dawnMode")){
    dawnMode = http.arg("dawnMode").toInt();
    saveDawnMmode();
  }
  
  routeGetAlarmConfig();
  
}

void routeGetAlarmConfig(){
  String out = "{";
  int _time;
    
  for (byte i = 0; i < 7; i++) {
    out += (alarm[i].state == true) ? "\"day_"+String(i)+"\":\"1\"," : "\"day_"+String(i)+"\":\"0\",";
    if(alarm[i].time){
      String h,m;
      h = (alarm[i].time/60<10) ? "0" + String(alarm[i].time/60) : String(alarm[i].time/60);
      m = (alarm[i].time%60<10) ? "0" + String(alarm[i].time%60) : String(alarm[i].time%60);
      out += "\"time_"+String(i)+"\":\""+h+":"+m+"\",";
    }else{
      out += "\"time_"+String(i)+"\":\""+timeClient.getHours()+":"+timeClient.getMinutes()+"\",";
    }
  }

  out += "\"dawnMode\":\"" + String(dawnMode) + "\"}";
  
  http.send(200, "text/json", out);
}

/**
 * главная страница
 */
void routeHome(){
  String out;
  
  out = "<form>";

      out += "<div class='ui-field-contain'>";
        out += "<label for='on'>Питание лампы:</label>";
        out += "<select name='on' id='on' data-role='slider' data-mini='true'>";
          out += "<option value='0'>Выкл</option>";
          out += "<option value='1'>Вкл</option>";
        out += "</select>";
      out += "</div>";
      
      out += "<div class='ui-field-contain'>";
        out += "<label for='currentMode'>Режим:</label>";
        out += "<select name='currentMode' id='currentMode' data-mini='true'>";
          
          out += "<option value='0'>Конфети</option>";
          out += "<option value='1'>Огонь</option>";
          out += "<option value='2'>Радуга вертикальная</option>";
          out += "<option value='3'>Радуга горизонтальная</option>";
          out += "<option value='4'>Смена цвета</option>";
          out += "<option value='5'>Безумие 3D</option>";
          out += "<option value='6'>Облака 3D</option>";
          out += "<option value='7'>Лава 3D</option>";
          out += "<option value='8'>Плазма 3D</option>";
          out += "<option value='9'>Радуга 3D</option>";
          out += "<option value='10'>Павлин 3D</option>";
          out += "<option value='11'>Зебра 3D</option>";
          out += "<option value='12'>Лес 3D</option>";
          out += "<option value='13'>Океан 3D</option>";
          out += "<option value='14'>Цвет</option>";
          out += "<option value='15'>Снегопад</option>";
          out += "<option value='16'>Матрица</option>";
          out += "<option value='17'>Светлячки</option>";
          
        out += "</select>";
      out += "</div>";
      
      out += "<div class='ui-field-contain'>";
        out += "<label for='brightness'>Яркость:</label>";
        out += "<input type='range' name='brightness' id='brightness' value='50' min='0' max='255' data-highlight='true'>";
      out += "</div>";
      
      out += "<div class='ui-field-contain'>";
        out += "<label for='speed'>Скорость:</label>";
        out += "<input type='range' name='speed' id='speed' value='50' min='0' max='100' data-highlight='true'>";
      out += "</div>";
      
      out += "<div class='ui-field-contain'>";
        out += "<label for='scale'>Масштаб:</label>";
        out += "<input type='range' name='scale' id='scale' value='50' min='0' max='100' data-highlight='true'>";
      out += "</div>";
      
    
  out += "</form>";
  out += "<script type='text/javascript'>$(()=>{syncConfig('/getconfig','/setconfig');});</script>";

  responseHtml(out);
}
