var deviceID = "300032001147343339383037";
var accessToken = "da837cbd013221af2cac61eea03e15d8459c49ea";
var funcName = "makeMove";
var rotationUnit = "degrees";
var noSelected = "none";

EventEnum = {
    COMPLETE : "complete",
    CALIBRATION_VALUES : "calibrationValues",
}

ButtonEnum = {
    FORWARD : {cmd: "forward", btnName: "forward-btn"},
    BACKWARD : {cmd: "backward", btnName: "backward-btn"},
    TURN_LEFT : {cmd: "turnLeft", btnName: "left-turn-btn"},
    TURN_RIGHT : {cmd: "turnRight", btnName: "right-turn-btn"},
    STOP : {cmd: "stop", btnName: "stop-btn"},
    SEND_SPEED : {cmd: "setSpeed", btnName: "send-speed-btn"},
    CAL_TURNING : {cmd: "calibrateTurning", btnName: "cal-turning-btn"},
    CAL_WHEELS : {cmd: "calibrateSpeed", btnName: "cal-wheels-btn"},
}

InputEnum = {
    DISTANCE : "distance-input",
    SPEED : "speed-input",
    ROTATION:  "rotation-input",
    UNIT : "unit-input",
    CAL_LEFT_WHEEL : "left-wheel-input",
    CAL_RIGHT_WHEEL : "right-wheel-input",
    CAL_TURNING : "turning-cal-input"
}

$(document).ready(function() {
    spark.login({accessToken: accessToken});
    spark.getDevice(deviceID).then(function(device) {
        device.onEvent(EventEnum.COMPLETE, function(data) {
            changeButtons(false);
        });
        device.onEvent(EventEnum.CALIBRATION_VALUES, function(data) {
            var array = base64js.toByteArray(data.data);
            outputCalibration(array);
        });
    });

    setTimeout(getCalibrationValues, 1000);
    
    for (var btnEnumStr in ButtonEnum) {
        var button = ButtonEnum[btnEnumStr];
        $("#" + button.btnName).click(function(event) {
            var name = event.target.id;
            
            if (name == ButtonEnum.SEND_SPEED.btnName && 
                getInput(InputEnum.SPEED) != NULL) {
                particleCall(getCmd(name), getInput(InputEnum.SPEED));
            }
            
            if (name == ButtonEnum.FORWARD.btnName ||
                name == ButtonEnum.BACKWARD.btnName) {
                var unit = getInput(InputEnum.UNIT);
                var distance = getInput(InputEnum.DISTANCE);
                if ((unit == null && distance != null) || 
                    (unit != null && distance == null)) return;
                particleCall(getCmd(name), distance, unit);
                changeButtons(true);
            }
            
            if (name == ButtonEnum.TURN_LEFT.btnName ||
                name == ButtonEnum.TURN_RIGHT.btnName) {
                var rotation = getInput(InputEnum.ROTATION);
                if (rotation == null) return;
                particleCall(getCmd(name), rotation, rotationUnit);
                changeButtons(true);
            }
            
            if (name == ButtonEnum.CAL_WHEELS.btnName) {
                var rightWheel = getInput(InputEnum.CAL_RIGHT_WHEEL);
                var leftWheel = getInput(InputEnum.CAL_LEFT_WHEEL);
                if (rightWheel == null || leftWheel == null) return;
                particleCall(getCmd(name), rightWheel, leftWheel);
            }
            
            if (name == ButtonEnum.CAL_TURNING.btnName) {
                particleCall(getCmd(name), getInput(InputEnum.CAL_TURNING));
            }
            
            if (name == ButtonEnum.STOP.btnName) {
                particleCall(getCmd(name));
                changeButtons(false);
            }
        });
    }
});

function outputCalibration(array) {
    setInput(InputEnum.SPEED, array[0] | (array[1] << 8));
    setInput(InputEnum.CAL_RIGHT_WHEEL, array[2] | (array[3] << 8));
    setInput(InputEnum.CAL_LEFT_WHEEL, array[4] | (array[5] << 8));
    setInput(InputEnum.CAL_TURNING, array[6] | (array[7] << 8));
    
}

function getCalibrationValues() {
    particleCall("sendCalibration");
}

function getCmd(btnName) {
    for (var btnEnumStr in ButtonEnum) {
        var button = ButtonEnum[btnEnumStr];
        if (button.btnName == btnName) {
            return button.cmd;
        }
    }
    return null;
}

function changeButtons(disabled) {
    for (var btnEnumStr in ButtonEnum) {
        var button = ButtonEnum[btnEnumStr];
        if (button.btnName == ButtonEnum.STOP.btnName) continue;
        
        $("#" + button.btnName).prop('disabled', disabled);
    }
}

function particleCall(cmd, parameters) {
    var data;
    if (particleCall.length == 1 && cmd != null) {
        data = cmd;
    } else {
        data = cmd;
        for (var arg = 1; arg < arguments.length; arg++) {
            if (arguments[arg] == null) continue;
            data += "," + arguments[arg];
        }
    }
    $.post("https://api.particle.io/v1/devices/"+deviceID+"/"+funcName, {arg: data, access_token: accessToken})
        .done(function(data) {
            
        });
}

function getInput(inputName) {
    var value;
    if (inputName == InputEnum.UNIT) {
        value = $("#" + inputName + " option:selected")[0].value;
        if (value == noSelected) {
            value = null;
        }
    } else {
        value = $("#" + inputName).val();
        if (value == "") value = null;
    }
    return value;
}

function setInput(inputName, value) {
    $("#" + inputName).val(value);
}