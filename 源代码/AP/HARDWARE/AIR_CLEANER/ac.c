#include "ac.h"

uint8_t PM_FAN_MODE=0; //风扇模式   0:自动(默认)    1:手动

static TaskHandle_t taskSensor_handle = NULL;
static TaskHandle_t taskFanGear_handle = NULL;
static TaskHandle_t taskKeyEnc_handle = NULL;
static TaskHandle_t taskWifiCloud_handle = NULL;
static TaskHandle_t taskNG_handle = NULL;
static TaskHandle_t Task_LvglDisp_handle = NULL;
static TaskHandle_t Task_LowPower_handle = NULL;

Purifier_State_t purifier;// 全局实例：空气净化器整体的状态全部保存在这个全局结构体里，到处都能用

/* -------------------- 6个任务函数前置声明 -------------------- */
// static静态函数只能在本文件ac.c内部可访问，外部main.c无法直接调用，起到封装效果
static void Task_Sensor(void *pvParameters);      //传感器采集任务函数
static void Task_FanGear(void *pvParameters);      //风扇PID控制任务函数
static void Task_KeyEnc(void *pvParameters);      //按键+旋转编码器扫描任务函数
static void Task_NG(void *pvParameters);      //按键+旋转编码器扫描任务函数
static void Task_WifiCloud(void *pvParameters);    //ESP8266和服务器通信任务函数
static void Task_LvglDisp(void *pvParameters);     //LCD/LVGL屏幕显示任务函数
static void Task_LowPower(void *pvParameters);     //按键和空闲的关机低功耗任务函数


int air()
{
	Pwm_Pm_Init();
	DWT_Init();
	PM_Init();
	Dht11_Init();
	ESP8266_Init();
	NG_Init();//负离子发生生器初始化
	PO_Init();//旋转编码器初始化
	MY_KEY_Init();

	printf("enter air\r\n");
	purifier.fan_speed=0;
	

		//===== 创建全局FreeRTOS任务 =====
    /*
    xTaskCreate 函数参数类型：
    1.任务函数入口地址
    2.任务名字符串，调试时用于标识任务，不影响功能
    3.任务栈大小，单位word(4字节)，栈越大局部变量可占用空间越多
    4.传给任务函数的参数，不需要时传NULL
    5.任务优先级，数值越大优先级越高
    6.任务句柄输出，需要删除/恢复任务时使用，暂不需要传NULL
    */
	/* NOTE: configMAX_PRIORITIES=5, valid prio is 0~4. The old prio 10 was
	   silently clamped to 4 by FreeRTOS, making Sensor EQUAL to FanPID(4)
	   and letting round-robin ticks preempt the PM2.5 timing window. */
	xTaskCreate((TaskFunction_t )Task_Sensor,(const char*)"Sensor",(uint16_t)512, (void*)NULL, (UBaseType_t)4, (TaskHandle_t*)&taskSensor_handle);
    xTaskCreate(Task_FanGear,    "FanGear",  512, NULL, 2, &taskFanGear_handle);
    xTaskCreate(Task_KeyEnc,    "KeyEnc",  512, NULL, 4, &taskKeyEnc_handle);
    xTaskCreate(Task_WifiCloud, "Wifi",    1024, NULL, 3, &taskWifiCloud_handle);
	xTaskCreate(Task_NG, "NG",256, NULL, 2, &taskNG_handle);
    xTaskCreate(Task_LvglDisp,  "LVGL",    1024,NULL, 1, &Task_LvglDisp_handle);
    xTaskCreate(Task_LowPower,  "LowPow",  256, NULL, 1, &Task_LowPower_handle);

	/* 启动任务调度器 */
	vTaskStartScheduler(); 		

	return 0;
}

/* --------------------------------------------------------------------- 任务具体实现 ---------------------------------------------------------------------- */
//传感器采集任务：读取温湿度 + PM2.5粉尘采集
static void Task_Sensor(void *pvParameters)
{
	printf("enter Task_Sensor\r\n");
    uint8_t temp, humi;
    int ret;
    uint8_t retry_cnt;
	
    while(1)
    {
		printf("PM_FAN_MODE= %d\r\n",PM_FAN_MODE);
		vTaskDelay(pdMS_TO_TICKS(10));
		
        retry_cnt = 0;
        do
        {
            ret = DHT11_GetData(&temp, &humi);
            retry_cnt++;
            if(ret != 0)
            {
                vTaskDelay(pdMS_TO_TICKS(20));
            }
        }while(ret != 0 && retry_cnt < 3);

        if(ret == 0)
        {
		//==== DHT11数据写入保护 ====
        taskENTER_CRITICAL();
            purifier.temp = temp;
            purifier.humi = humi;
		taskEXIT_CRITICAL();
		printf("温度:%d 度 ,湿度:%d %% \r\n",purifier.temp, purifier.humi);
        }else
        {
            //printf("DHT11读取失败!\r\n");
        }

		vTaskDelay(pdMS_TO_TICKS(20));
		
        //========PM2.5粉尘采集========
        /* ADC1 is enabled ONCE in ADC_PM_Init(); do NOT toggle ADON per read,
           the first conversion after re-enable is unreliable */

        Get_PM_Value_ToFreeRtos(&purifier);

		//调试性打印，确认后可删除
		printf("PM2.5 = %d ug/m3\r\n", purifier.pm25_value);



        //1秒采集一次，DHT11最小间隔1s。
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}



//风扇控制任务，优先级较高：根据粉尘浓度进行风扇接口控制
static void Task_FanGear(void *pvParameters)
{
		uint16_t pm_val;
		static uint16_t fan_last_ccr=0;
		uint16_t ccr;
		uint16_t F=0;	//标志位
		int16_t temp_enc;//旋转编码值备份
		//printf("");
    while(1)
    {
			F=0;
		
			if(!PM_FAN_MODE){//风扇自动模式
			//==== PM 数据读保护 ====
			taskENTER_CRITICAL();
			pm_val = purifier.pm25_value; //读取当前灰尘
			taskEXIT_CRITICAL();
			
			// 判断灰尘浓度区间，分配风扇档位
			if(pm_val == 0)   //灰尘0‑50：空气很干净
			{
					ccr = 0;      //风扇停止，省电静音
			}
			else if(pm_val < 200) //灰尘50‑200：轻度污染
			{
					ccr = 3800;  //低速吹风
			}
			else if(pm_val < 500) //灰尘200‑500：中度污染
			{
					ccr = 5000;  //中速吹风
			}
			else                  //灰尘500‑1000：重度污染
			{
					ccr = 9999;  //风扇全速强力净化
			}
			
			if(ccr==0||fan_last_ccr!=ccr ){
			fan_last_ccr=ccr;
			vTaskDelay(pdMS_TO_TICKS(50));
			TIM_SetCompare2(TIM4, ccr); //输出PWM
		}
		}else if(PM_FAN_MODE)//风扇手动模式
			{
				fan_last_ccr = 0;
				
				taskENTER_CRITICAL();
				temp_enc=purifier.fan_speed;
				taskEXIT_CRITICAL();
				vTaskDelay(pdMS_TO_TICKS(50));
				TIM_SetCompare2(TIM4, temp_enc); //输出PWM
			}
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}



//按键+旋转编码器扫描任务函数
static void Task_KeyEnc(void *pvParameters)
{
	int16_t te;
	static int16_t last_enc = -1;   // 上次编码器值
    while(1)
    {
        //扫描按键并修改净化器工作模式
				KEY_To_PMpj();

        //读取编码器旋转值
			taskENTER_CRITICAL();
		//这里读取，绝对不会被编码器中断打断
			te= encoder_cnt;
			taskEXIT_CRITICAL();

		/* 只在编码器值发生变化时才更新fan_speed！
		   原来20ms无条件用编码器值覆盖fan_speed，
		   云端下发"on"设置的9999会在20ms内被编码器旧值(0)覆盖，
		   导致风扇永远打不开 */
		if(te != last_enc)
		{
			last_enc = te;
			purifier.fan_speed=(te==0)?0:(te*200-1);
		}

			vTaskDelay(pdMS_TO_TICKS(30));
    }
}


//WiFi和服务器通信任务：上传数据，接收手机页面下发的指令
//WiFi和服务器通信任务：上传数据，接收手机页面下发的指令
static void Task_WifiCloud(void *pvParameters)
{
    uint16_t report_timer = 0;   // 上报计时器，500ms计数一次

    // 1. 初始化ESP8266串口
    //ESP8266_Init();

    // ===== 首次联网：WiFi → TCP → MQTT =====
    while(ESP8266_ConnectWIFI() != ESP_OK)
    {
        printf("WiFi连接失败，重试中...\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    while(ESP8266_ConnectTCP() != ESP_OK)
    {
        //printf("TCP连接失败，重试中...\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    while(ESP8266_ConnectMQTT() != ESP_OK)
    {
        printf("MQTT连接失败，重试中...\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    // 2. 订阅风扇 + 负离子控制主题（接收云端控制指令）
    // 温湿度、PM2.5只上报不订阅，不接收控制
    // 两次订阅之间必须留间隔：上一条CIPSEND的数据还没发完时，
    // 下一条AT+CIPSEND会被ESP8266当作数据拼进上个报文，导致订阅报文损坏
    ESP8266_MQTTSubscribe(TOPIC_Fan);
    vTaskDelay(pdMS_TO_TICKS(200));
    ESP8266_MQTTSubscribe(TOPIC_NG);
    printf("云端通信任务启动完成\r\n");

    while(1)
    {
        // ========== 优先级1：实时处理云端下发指令 ==========
        // 必须每次循环都调用，防止串口接收缓冲区溢出
        ESP8266_ProcessData();

        // ========== 优先级2：断线自动重连 ==========
        if(MQTT_Connected == 0)
        {
            printf("MQTT断开，开始逐级重连...\r\n");

            if(ESP8266_ConnectWIFI() == ESP_OK)
            {
                if(ESP8266_ConnectTCP() == ESP_OK)
                {
                    if(ESP8266_ConnectMQTT() == ESP_OK)
                    {
                        // 重连成功后重新订阅所有控制主题
                        ESP8266_MQTTSubscribe(TOPIC_Fan);
                        vTaskDelay(pdMS_TO_TICKS(200));
                        ESP8266_MQTTSubscribe(TOPIC_NG);
                        printf("云端重连成功，已重新订阅\r\n");
                    }
                }
            }
            // 重连间隔拉长，避免频繁刷屏
            vTaskDelay(pdMS_TO_TICKS(3000));
            continue;
        }

        // ========== 优先级3：定时上报所有设备数据 ==========
        report_timer++;
        if(report_timer >= 10) // 500ms × 10 = 5秒上报一次
        {
            report_timer = 0;
            char buf[32] = {0};
            uint16_t pm, fspd;
            uint8_t  tp, hm;

            // 读取共享数据必须加临界区，防止与 Sensor/KeyEnc 任务竞争
            taskENTER_CRITICAL();
            pm   = purifier.pm25_value;
            tp   = purifier.temp;
            hm   = purifier.humi;
            fspd = purifier.fan_speed;
            taskEXIT_CRITICAL();

            // PM2.5 数值限幅，防止异常值(如8680)上报到云端
            if(pm > 1000) pm = 1000;

            // ---------- 1. 上报风扇状态（严格按你的格式） ----------
            if(PM_FAN_MODE == 0) // 自动模式
            {
                ESP8266_MQTTPublish(TOPIC_Fan, "auto");
            }
            else // 手动模式
            {
                if(fspd > 0)
                {
                    ESP8266_MQTTPublish(TOPIC_Fan, "手动:on");
                }
                else
                {
                    ESP8266_MQTTPublish(TOPIC_Fan, "手动:off");
                }
            }

            // ---------- 2. 上报温度 → temp004（带单位℃） ----------
            snprintf(buf, sizeof(buf), "%d\xc2\xb0""C", tp);
            ESP8266_MQTTPublish(TOPIC_Tem, buf);

            // ---------- 3. 上报湿度 → humidity004（带单位%） ----------
            snprintf(buf, sizeof(buf), "%d%%", hm);
            ESP8266_MQTTPublish(TOPIC_Hum, buf);

            // ---------- 4. 上报PM2.5 → pm004（带单位ug/m3） ----------
            snprintf(buf, sizeof(buf), "%d ug/m3", pm);
            ESP8266_MQTTPublish(TOPIC_Pm, buf);
        }

        // ========== 优先级4：MQTT 保活心跳（每30秒一次） ==========
        // keepalive=60s，30s发一次PINGREQ留足余量，防止服务器因超时踢线
        {
            static uint16_t ping_timer = 0;
            if(++ping_timer >= 60)  // 500ms × 60 = 30秒
            {
                ping_timer = 0;
                if(ESP8266_MQTTPing() != ESP_OK)
                {
                    printf("MQTT PING 失败，连接可能已断开\r\n");
                }
            }
        }

        // ========== 优先级5：定期重新订阅（每60秒一次） ==========
        // 巴法云有时会出现"设备在线但未订阅"的情况，
        // 定期重发SUBSCRIBE可以兜底，保证云端指令能正常下发
        {
            static uint16_t sub_timer = 0;
            if(++sub_timer >= 120)  // 500ms × 120 = 60秒
            {
                sub_timer = 0;
                ESP8266_MQTTSubscribe(TOPIC_Fan);
                vTaskDelay(pdMS_TO_TICKS(200));
                ESP8266_MQTTSubscribe(TOPIC_NG);
            }
        }

        // 任务调度周期：500ms
        // 兼顾云端指令响应速度 + CPU占用率
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

//负离子发生器控制任务
//自动模式(PM_FAN_MODE=0)：按PM浓度自动开关，向NG006上报"auto"
//手动模式(PM_FAN_MODE=1)：由云端/按键控制开关，向NG006上报"on"/"off"
static void Task_NG(void *pvParameters)
{
    uint8_t last_mode = 0xFF;    // 上次上报的模式，0xFF初始值，上电强制上报一次
    uint8_t last_state = 0xFF;   // 上次上报的引脚状态
    uint16_t keepalive = 0;

    while(1)
    {
        uint8_t mode = PM_FAN_MODE;
        uint8_t st = Read_stat_NG();

        // ========== 自动模式逻辑：按PM浓度自动开关 ==========
        if(mode == 0)
        {
            uint16_t pm_now;
            taskENTER_CRITICAL();
            pm_now = purifier.pm25_value;
            taskEXIT_CRITICAL();
            // PM2.5 ≥ AG_TARGET_VALUE 开启负离子，否则关闭
            Control_NG((pm_now >= AG_TARGET_VALUE) ? 1 : 0);
            st = Read_stat_NG();
        }
        // ========== 手动模式逻辑 ==========
        // 引脚由云端指令(Control_NG)/按键(Switch_status_NG)控制，本任务只读取和上报

        // ========== 上报NG006：自动模式发"auto"，手动模式发"on"/"off" ==========
        // 模式或状态变化立即上报，另外每30秒保活上报一次(500ms×60)
        if(mode != last_mode || st != last_state || ++keepalive >= 60)
        {
            keepalive = 0;
            if(mode == 0)
            {
                ESP8266_MQTTPublish(TOPIC_NG, "auto");
            }
            else
            {
                ESP8266_MQTTPublish(TOPIC_NG, (st == 1) ? "on" : "off");
            }
            last_mode = mode;
            last_state = st;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}




// 声明一个自定义的 TTF 字体
LV_FONT_DECLARE(my_font);

/* ===== LVGL 界面控件句柄(供刷新循环与回调访问) ===== */
static lv_obj_t *label_temp;     // 温度值
static lv_obj_t *label_humi;     // 湿度值
static lv_obj_t *label_pm;       // PM2.5值
static lv_obj_t *label_fan_st;   // 风扇状态文本
static lv_obj_t *label_ng_st;    // 负离子状态文本
static lv_obj_t *btn_mode;       // 自动/手动切换按钮
static lv_obj_t *btn_fan;        // 手动模式:风扇开关
static lv_obj_t *btn_ng;         // 手动模式:负离子开关
static lv_obj_t *label_mode_btn; // 模式按钮文字
static lv_obj_t *label_fan_btn;  // 风扇按钮文字
static lv_obj_t *label_ng_btn;   // 负离子按钮文字

/* 刷新所有显示内容:传感器数值 + 风扇/负离子状态 + 按钮可见性 */
static void ui_update(void)
{
    char buf[32];
    uint8_t tp, hm;
    uint16_t pm;
    uint8_t mode;

    taskENTER_CRITICAL();
    tp   = purifier.temp;
    hm   = purifier.humi;
    pm   = purifier.pm25_value;
    taskEXIT_CRITICAL();
    mode = PM_FAN_MODE;

    snprintf(buf, sizeof(buf), "%d ℃ ", tp);
    lv_label_set_text(label_temp, buf);
    snprintf(buf, sizeof(buf), "%d %%", hm);
    lv_label_set_text(label_humi, buf);
    snprintf(buf, sizeof(buf), "%d ug/m3", pm);
    lv_label_set_text(label_pm, buf);

    if(mode == 0)          /* 自动模式:风扇与负离子均显示"自动",隐藏手动按钮 */
    {
        lv_label_set_text(label_fan_st, "自动");
        lv_label_set_text(label_ng_st, "自动");
        lv_label_set_text(label_mode_btn, "切换手动");
        lv_obj_add_flag(btn_fan, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_ng,  LV_OBJ_FLAG_HIDDEN);
    }
    else                    /* 手动模式:显示开/关,展示控制按钮 */
    {
        uint8_t fan_on = (purifier.fan_speed > 0);
        uint8_t ng_on  = Read_stat_NG();
        lv_label_set_text(label_fan_st, fan_on ? "开 " : "关 ");
        lv_label_set_text(label_ng_st,  ng_on  ? "开 " : "关 ");
        lv_label_set_text(label_mode_btn, "切换自动");
        lv_obj_clear_flag(btn_fan, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(btn_ng,  LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(label_fan_btn, fan_on ? "风扇:关 " : "风扇:开 ");
        lv_label_set_text(label_ng_btn,  ng_on  ? "负离子:关 " : "负离子:开 ");
    }
}

/* 模式切换:自动<->手动。切到手动时让fan_speed匹配当前PWM实际状态 */
static void cb_mode(lv_event_t *e)
{
    if(PM_FAN_MODE == 0)
    {
        PM_FAN_MODE = 1;
        purifier.fan_speed = (TIM4->CCR2 > 0) ? 9999 : 0;
    }
    else
    {
        PM_FAN_MODE = 0;
    }
    ui_update();
}

/* 手动模式下风扇开关:开=PWM最大值9999, 关=0 */
static void cb_fan(lv_event_t *e)
{
    if(PM_FAN_MODE != 1) return;
    purifier.fan_speed = (purifier.fan_speed > 0) ? 0 : 9999;
    ui_update();
}

/* 手动模式下负离子开关 */
static void cb_ng(lv_event_t *e)
{
    if(PM_FAN_MODE != 1) return;
    Control_NG(Read_stat_NG() ? 0 : 1);
    ui_update();
}

//屏幕刷新任务，移植LVGL图形库，读取结构体数据刷新界面
static void Task_LvglDisp(void *pvParameters)
{
	// 初始化lvgl
    lv_init();

    // 初始化lvgl显示设备
    lv_port_disp_init();

    // 初始化lvgl输入设备
    lv_port_indev_init();

    //tim3初始化，定时周期为1ms
    TIM3_Init();

    /**************** 自定义界面 ****************/
    lv_obj_t *scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), 0);   /* 纯白背景 */
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* ---------- 标题 ---------- */
    lv_obj_t *title = lv_label_create(scr);
    lv_obj_set_style_text_font(title, &my_font, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x000000), 0);
    lv_label_set_text(title, "空气净化器");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 8);

    /* ---------- 传感器数据区 ----------
       第一行: 温度 XX ℃      湿度 XX %
       第二行: PM2.5 XX ug/m3                      */
    lv_obj_t *t1 = lv_label_create(scr);
    lv_obj_set_style_text_font(t1, &my_font, 0);
    lv_obj_set_style_text_color(t1, lv_color_hex(0x000000), 0);
    lv_label_set_text(t1, "温度:");
    lv_obj_align(t1, LV_ALIGN_TOP_LEFT, 30, 50);

    label_temp = lv_label_create(scr);
    lv_obj_set_style_text_font(label_temp, &my_font, 0);
    lv_obj_set_style_text_color(label_temp, lv_color_hex(0x000000), 0);
    lv_label_set_text(label_temp, "-- ℃ ");
    lv_obj_align_to(label_temp, t1, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    lv_obj_t *t2 = lv_label_create(scr);
    lv_obj_set_style_text_font(t2, &my_font, 0);
    lv_obj_set_style_text_color(t2, lv_color_hex(0x000000), 0);
    lv_label_set_text(t2, "湿度:");
    lv_obj_align(t2, LV_ALIGN_TOP_MID, 30, 50);

    label_humi = lv_label_create(scr);
    lv_obj_set_style_text_font(label_humi, &my_font, 0);
    lv_obj_set_style_text_color(label_humi, lv_color_hex(0x000000), 0);
    lv_label_set_text(label_humi, "-- %");
    lv_obj_align_to(label_humi, t2, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    lv_obj_t *t3 = lv_label_create(scr);
    lv_obj_set_style_text_font(t3, &my_font, 0);
    lv_obj_set_style_text_color(t3, lv_color_hex(0x000000), 0);
    lv_label_set_text(t3, "PM2.5:");
    lv_obj_align(t3, LV_ALIGN_TOP_LEFT, 30, 82);

    label_pm = lv_label_create(scr);
    lv_obj_set_style_text_font(label_pm, &my_font, 0);
    lv_obj_set_style_text_color(label_pm, lv_color_hex(0x000000), 0);
    lv_label_set_text(label_pm, "-- ug/m3");
    lv_obj_align_to(label_pm, t3, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    /* ---------- 风扇/负离子 状态显示区 ---------- */
    lv_obj_t *sf = lv_label_create(scr);
    lv_obj_set_style_text_font(sf, &my_font, 0);
    lv_obj_set_style_text_color(sf, lv_color_hex(0x000000), 0);
    lv_label_set_text(sf, "风扇:");
    lv_obj_align(sf, LV_ALIGN_TOP_LEFT, 30, 130);

    label_fan_st = lv_label_create(scr);
    lv_obj_set_style_text_font(label_fan_st, &my_font, 0);
    lv_obj_set_style_text_color(label_fan_st, lv_color_hex(0x000000), 0);
    lv_label_set_text(label_fan_st, "--");
    lv_obj_align_to(label_fan_st, sf, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    lv_obj_t *sn = lv_label_create(scr);
    lv_obj_set_style_text_font(sn, &my_font, 0);
    lv_obj_set_style_text_color(sn, lv_color_hex(0x000000), 0);
    lv_label_set_text(sn, "负离子:");
    lv_obj_align(sn, LV_ALIGN_TOP_MID, 30, 130);

    label_ng_st = lv_label_create(scr);
    lv_obj_set_style_text_font(label_ng_st, &my_font, 0);
    lv_obj_set_style_text_color(label_ng_st, lv_color_hex(0x000000), 0);
    lv_label_set_text(label_ng_st, "--");
    lv_obj_align_to(label_ng_st, sn, LV_ALIGN_OUT_RIGHT_MID, 6, 0);

    /* ---------- 按钮区 ----------
       统一风格: 浅灰底(0xE8E8E8) + 黑字, 90x36
       自动模式: 只显示中间[切换手动]
       手动模式: 左[风扇] 中[切换自动] 右[负离子] 三个并排 */

    /* 风扇开关按钮(手动模式显示) */
    btn_fan = lv_btn_create(scr);
    lv_obj_set_size(btn_fan, 90, 36);
    lv_obj_set_style_bg_color(btn_fan, lv_color_hex(0x3498DB), 0);
    lv_obj_set_style_bg_opa(btn_fan, LV_OPA_COVER, 0);
    lv_obj_align(btn_fan, LV_ALIGN_BOTTOM_LEFT, 15, -15);
    lv_obj_add_event_cb(btn_fan, cb_fan, LV_EVENT_CLICKED, NULL);
    label_fan_btn = lv_label_create(btn_fan);
    lv_obj_set_style_text_font(label_fan_btn, &my_font, 0);
    lv_obj_set_style_text_color(label_fan_btn, lv_color_hex(0x000000), 0);
    lv_label_set_text(label_fan_btn, "风扇:开");
    lv_obj_center(label_fan_btn);
    lv_obj_add_flag(btn_fan, LV_OBJ_FLAG_HIDDEN);

    /* 模式切换按钮(始终显示) */
    btn_mode = lv_btn_create(scr);
    lv_obj_set_size(btn_mode, 90, 36);
    lv_obj_set_style_bg_color(btn_mode, lv_color_hex(0x3498DB), 0);
    lv_obj_set_style_bg_opa(btn_mode, LV_OPA_COVER, 0);
    lv_obj_align(btn_mode, LV_ALIGN_BOTTOM_MID, 0, -15);
    lv_obj_add_event_cb(btn_mode, cb_mode, LV_EVENT_CLICKED, NULL);
    label_mode_btn = lv_label_create(btn_mode);
    lv_obj_set_style_text_font(label_mode_btn, &my_font, 0);
    lv_obj_set_style_text_color(label_mode_btn, lv_color_hex(0x000000), 0);
    lv_label_set_text(label_mode_btn, "切换手动");
    lv_obj_center(label_mode_btn);

    /* 负离子开关按钮(手动模式显示) */
    btn_ng = lv_btn_create(scr);
    lv_obj_set_size(btn_ng, 90, 36);
    lv_obj_set_style_bg_color(btn_ng, lv_color_hex(0x3498DB), 0);
    lv_obj_set_style_bg_opa(btn_ng, LV_OPA_COVER, 0);
    lv_obj_align(btn_ng, LV_ALIGN_BOTTOM_RIGHT, -15, -15);
    lv_obj_add_event_cb(btn_ng, cb_ng, LV_EVENT_CLICKED, NULL);
    label_ng_btn = lv_label_create(btn_ng);
    lv_obj_set_style_text_font(label_ng_btn, &my_font, 0);
    lv_obj_set_style_text_color(label_ng_btn, lv_color_hex(0x000000), 0);
    lv_label_set_text(label_ng_btn, "负离子:开");
    lv_obj_center(label_ng_btn);
    lv_obj_add_flag(btn_ng, LV_OBJ_FLAG_HIDDEN);

    ui_update();   /* 首次刷新,避免上电显示"--"太久 */

    uint32_t last_tick = 0;
    while(1)
    {
        lv_task_handler();
        /* 每300ms刷新一次传感器数值与状态显示 */
        if((uint32_t)(xTaskGetTickCount() - last_tick) >= pdMS_TO_TICKS(300))
        {
            last_tick = xTaskGetTickCount();
            ui_update();
        }
        DWT_Delay_ms(5);
    }
}


//按键和空闲的功耗管理任务：计时用户无操作时间，自动切换省电挡位
//策略: 30s无操作关背光; 60s无操作切自动模式+风扇低速; PM超标强制唤醒
static void Task_LowPower(void *pvParameters)
{
    uint16_t idle_sec = 0;       // 空闲累计秒数
    uint8_t  backlight_off = 0;  // 背光是否已关

    while(1)
    {
        /* PM2.5超标强制唤醒: 即使空闲, 空气差时不能省电 */
        if(purifier.pm25_value > 500)
        {
            idle_sec = 0;
            if(backlight_off)
            {
                LCD_LED = 1;              /* 开背光 */
                purifier.lcd_backlight = 1;
                backlight_off = 0;
            }
            PM_FAN_MODE = 0;              /* 切自动模式, FanGear任务会全速运转 */
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        /* 检查用户是否操作了(按键/触屏/云端) */
        if(purifier.user_action_flag)
        {
            purifier.user_action_flag = 0;
            idle_sec = 0;
            if(backlight_off)
            {
                LCD_LED = 1;              /* 唤醒: 开背光 */
                purifier.lcd_backlight = 1;
                backlight_off = 0;
            }
        }
        else
        {
            idle_sec++;

            /* 30秒无操作: 关LCD背光省电 */
            if(idle_sec >= 30 && !backlight_off)
            {
                LCD_LED = 0;
                purifier.lcd_backlight = 0;
                backlight_off = 1;
            }

            /* 60秒无操作: 切自动模式, 交给FanGear自动降速 */
            if(idle_sec >= 60)
            {
                PM_FAN_MODE = 0;          /* 切自动模式, 风扇按PM浓度自动调速 */
                idle_sec = 60;            /* 防止溢出, 保持60秒状态 */
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


/* -------------------- 以下为备用低功耗钩子 -------------------- 
// 当所有任务都处于阻塞状态时，CPU没有可运行的任务；空闲钩子函数执行WFI进入睡眠模式省电。
void vApplicationIdleHook(void)
{
    __WFI();
    // WFI指令：等待中断，CPU进入睡眠模式，任意中断的发生都会唤醒CPU
}
*/

/**
 * @brief  FreeRTOS任务调用的PM2.5采集函数
 * @param  purifier:净化器状态结构体指针，用于写入pm25_value
 */
void Get_PM_Value_ToFreeRtos(Purifier_State_t *xxx)
{
		uint16_t ADCVal;
		int dustVal = 0;
		float Voltage;
		uint16_t filter_out;
		uint16_t voltage_mv;
	
		taskENTER_CRITICAL();
		ADCVal=Acquisition_timing();
		taskEXIT_CRITICAL();	

	
		//printf("采集当前范围ADC值=%d\r\n",ADCVal);
		printf("ADC:%d \r\n",ADCVal);
	
		//----------去极值平均滤波 ----------
		filter_out = Filter_ExtremeAverage(ADCVal);
		//filter_out =ADCVal;


		//---------- 电压换算 (mV) ----------
		// 注意：基准转换影响分压 *11 和不要*2，选择一种
		voltage_mv = (u16)((( (uint32_t)filter_out * 3300 ) / 4096 ));
		Voltage = voltage_mv / 1000.0f;

		//---------- 灰尘浓度计算，参考原理图逻辑 ----------
		if(voltage_mv > NO_DUST_MV)
		{
			dustVal = (int)( (voltage_mv - NO_DUST_MV) * COV_RATIO );
		}
		else
		{
			dustVal = 0;
		}

		// 安全限幅：防止异常值(负数或超大值)写入uint16_t后变成乱码
		//if(dustVal < 0)       dustVal = 0;
		//if(dustVal > 1000)    dustVal = 1000;
		taskENTER_CRITICAL();
		xxx->pm25_value=dustVal;
		taskEXIT_CRITICAL();
		
		//printf("ADC:%d  Filter:%d  Voltage:%.2f V  PM2.5:%d ug/m3\r\n",ADCVal,filter_out,Voltage,dustVal);
		
	
}

//针对巴法云传过来的数据设置 风扇模式和开关状态
void Bafa_Data_set_moode1(void){
	PM_FAN_MODE=1;//设置手动模式标志位
	purifier.fan_speed=9999;//改变结构体数值（pwm占空比），间接控制风扇开关，最大转
	ESP8266_MQTTPublish(TOPIC_Fan, "手动:on");
}
void Bafa_Data_set_moode2(void){
	PM_FAN_MODE=1;//设置手动模式标志位
	purifier.fan_speed=0;//改变结构体数值（pwm占空比），间接控制风扇开关
	ESP8266_MQTTPublish(TOPIC_Fan, "手动:off");
}
void Bafa_Data_set_auto(void){
	PM_FAN_MODE=0;//设置自动模式标志位
	ESP8266_MQTTPublish(TOPIC_Fan, "auto");
} 
//设置用户是否正在使用标记,参数是保留功能，暂无使用
void Set_user_act(uint8_t u){

	if(!purifier.user_action_flag)
		purifier.user_action_flag=1;

};
