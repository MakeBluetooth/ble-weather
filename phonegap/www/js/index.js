var THERMOMETER_SERVICE = 'BBB0';
var TEMPERATURE_CHARACTERISTIC = 'BBB1';
var HUMIDITY_CHARACTERISTIC = 'BBB2';
var PRESSURE_CHARACTERISTIC = 'BBB3';

var app = {
    initialize: function() {
        this.bindEvents();
        this.showMainPage();
    },
    bindEvents: function() {
        document.addEventListener('deviceready', this.onDeviceReady, false);
        deviceList.addEventListener('click', this.connect, false);
        refreshButton.addEventListener('click', this.refreshDeviceList, false);
        disconnectButton.addEventListener('click', this.disconnect, false);
    },
    onDeviceReady: function() {
        app.refreshDeviceList();
    },
    refreshDeviceList: function() {
        deviceList.innerHTML = ''; // empty the list
        ble.scan([THERMOMETER_SERVICE], 5, app.onDiscoverDevice, app.onError);
    },
    onDiscoverDevice: function(device) {
       var listItem = document.createElement('li');
       listItem.innerHTML = device.name + ' ' + device.id;
       listItem.dataset.deviceId = device.id;
       deviceList.appendChild(listItem);
    },
    connect: function(e) {
        var deviceId = e.target.dataset.deviceId;
        ble.connect(deviceId, app.onConnect, app.onError);
    },
    onConnect: function(peripheral) {
        app.peripheral = peripheral;
        app.showDetailPage();

        var failure = function(reason) {
           navigator.notification.alert(reason, null, "Temperature Error");
        };

        // subscribe to be notified when the button state changes
        ble.startNotification(
            peripheral.id,
            THERMOMETER_SERVICE,
            TEMPERATURE_CHARACTERISTIC,
            app.onTemperatureChange,
            failure
        );

        // subscribe to be notified when the button state changes
        ble.startNotification(
            peripheral.id,
            THERMOMETER_SERVICE,
            HUMIDITY_CHARACTERISTIC,
            app.onHumidityChange,
            failure
        );

        // subscribe to be notified when the button state changes
        ble.startNotification(
            peripheral.id,
            THERMOMETER_SERVICE,
            PRESSURE_CHARACTERISTIC,
            app.onPressureChange,
            failure
        );

        // read the initial value
        ble.read(
            peripheral.id,
            THERMOMETER_SERVICE,
            TEMPERATURE_CHARACTERISTIC,
            app.onTemperatureChange,
            failure
        );

        ble.read(
            peripheral.id,
            THERMOMETER_SERVICE,
            HUMIDITY_CHARACTERISTIC,
            app.onHumidityChange,
            failure
        );

        ble.read(
            peripheral.id,
            THERMOMETER_SERVICE,
            PRESSURE_CHARACTERISTIC,
            app.onPressureChange,
            failure
        );

    },
    onTemperatureChange: function(buffer) {
        var data = new Float32Array(buffer);
        var temperature = data[0];
        var message = "Temperature is " + temperature.toFixed(1) + "&deg;F";
        temperatureDiv.innerHTML = message;
    },
    onHumidityChange: function(buffer) {
        var data = new Float32Array(buffer);
        var humidity = data[0];
        var message = "Humidity is " + humidity.toFixed(1) + "%";
        humidityDiv.innerHTML = message;
    },
    onPressureChange: function(buffer) {
        var data = new Int32Array(buffer);
        var pressure = data[0];
        // http://www.srh.noaa.gov/images/epz/wxcalc/pressureConversion.pdf
        var pressureInHg = pressure * 0.0295300 / 100;
        var message = "Pressure is " + pressure + " pascal<br/>" +
          "Pressure is " + pressureInHg.toFixed(2) + " inHg";
        pressureDiv.innerHTML = message;
    },
    disconnect: function(e) {
        if (app.peripheral && app.peripheral.id) {
            ble.disconnect(app.peripheral.id, app.showMainPage, app.onError);
        }
    },
    showMainPage: function() {
        mainPage.hidden = false;
        detailPage.hidden = true;
    },
    showDetailPage: function() {
        mainPage.hidden = true;
        detailPage.hidden = false;
    },
    onError: function(reason) {
       navigator.notification.alert(reason, app.showMainPage, "Error");
    }
};
