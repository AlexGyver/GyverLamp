void webserver(){
  
  http.onNotFound(routeNotFound);

  /** главная */
  http.on("/", routeHome); 
  /** прием конфигурации */
  http.on("/setconfig", routeSetConfig); 
  /** получить текущие настройки/конфигурацию */
  http.on("/getconfig", routeGetConfig); 
  /** прием конфигурации таймера вкл/выкл */
  //http.on("/settimerconfig", routeGetTimerConfig); 
  /** получить текущие настройки/конфигурацию таймера вкл/выкл */
  //http.on("/gettimerconfig", routeSetTimerConfig); 
  
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
      html += "<div data-role='header' class='jqm-header'><h2><a class='ui-link' data-ajax='false' href='/'>AlexGyver Lamp</a></h2></div>";
      html += "<div role='main' class='ui-content jqm-content'>";
      html += "";
      html += out;
      html += "";
      html += "</div>";
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
//  out += "" + include;
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
  
  out += "    <script type=\"text/javascript\">\n";
  out += "        $.ajax({url: '/getconfig', dataType:'json', success: init});\n";
  out += "        //let config = {\"currentMode\":\"4\",\"brightness\":\"255\",\"speed\":\"30\",\"scale\":\"40\",\"on\":\"1\"};\n";
  out += "        init(config);\n";
  out += "        function init(config){\n";
  out += "            /**\n";
  out += "             * костыль/фича\n";
  out += "             * лень было искать как нормально установить параметы в виджеты\n";
  out += "             * пока не установятся параметры из ESP, отправка не будет осуществляться\n";
  out += "             * @type {boolean}\n";
  out += "             */\n";
  out += "            window.changeReaction = false;\n";
  out += "            $(document).ready(() => {\n";
  out += "                let timeouts = {};\n";
  out += "                $('select, input').on('change',(v) => {\n";
  out += "                    /** если данные не пришли, ничего не отправляем/сохраненяем */\n";
  out += "                    if(window.changeReaction === false) return;\n";
  out += "                    let that = $(v.currentTarget), name = that.attr('name'), outData = {};\n";
  out += "                    /** если в очереди на отправку/сохранение есть такой параметр, то снимим предыдущее значение */\n";
  out += "                    if(timeouts[name] != undefined)\n";
  out += "                        clearTimeout(timeouts[name]);\n";
  out += "                    /**\n";
  out += "                     * установим измененный параметр в очередь на отправку/сохранение в ESP\n";
  out += "                     * @type {number}\n";
  out += "                     */\n";
  out += "                    timeouts[name] = setTimeout(() => {\n";
  out += "                        window.changeReaction = false;\n";
  out += "                        outData[name] = that.val();\n";
  out += "                        $.ajax({\n";
  out += "                            url:'setconfig',\n";
  out += "                            data: outData,\n";
  out += "                            dataType: 'json',\n";
  out += "                            success: (json) => {\n";
  out += "                                if(json[name] = outData[name])\n";
  out += "                                    window.changeReaction = true;\n";
  out += "                                else\n";
  out += "                                    alert('Не удалось сохранить настройки.');\n";
  out += "                                setConfig(json);\n";
  out += "                            },\n";
  out += "                            error: (event) => alert(`Не удалось сохранить настройки.\nПроизошла ошибка \"${event.status} ${event.statusText}\".`)\n";
  out += "                        });\n";
  out += "                    }, 500);\n";
  out += "                });\n";
  out += "                setConfig(config);\n";
  out += "                /** установим актуальные параметры из ESP */\n";
  out += "                function setConfig(config){\n";
  out += "                  window.changeReaction = false;\n";
  out += "                  for (let key in config){\n";
  out += "                      let that = $(`[name=${key}]`);\n";
  out += "                      that.val(config[key]);\n";
  out += "                      that.trigger('change');\n";
  out += "                  }\n";
  out += "                  /**\n";
  out += "                   * разрешаем вносить изменеия в конфигу\n";
  out += "                   * @type {boolean}\n";
  out += "                   */\n";
  out += "                  window.changeReaction = true;\n";
  out += "                }\n";
  out += "            });\n";
  out += "        }\n";
  out += "    </script>\n";

  responseHtml(out);
}
