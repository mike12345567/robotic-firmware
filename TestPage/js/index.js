var deviceID = "300032001147343339383037";
var funcName = "makeMove";
var rotationUnit = "degrees";
var noSelected = "none";
var waitForLift = false;
var maxGyroRead = 22000;
var wasFail = false;

EventEnum = {
    COMPLETE : "complete",
    CALIBRATION_VALUES : "calibrationValues",
    ULTRASONIC_VALUES: "distanceCm",
    STOPPED : "stopped",
    FAILED : "failed",
    HAS_FAILED : "hasFailed",
    GYROSCOPE_READINGS : "gyroscopeReadings",
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
    CAL_FRICTION : {cmd: "calibrateFriction", btnName: "cal-friction-btn"},
    RESET_FAIL : {cmd: "resetFailed", btnName: "cal-reset-fail-btn"},
}

JoystickEnum = {
    JOY_FWD : { cmd: "forward", btnName: "joy-fwd-btn"},
    JOY_LEFT : { cmd: "turnLeft", btnName: "joy-left-btn"},
    JOY_RIGHT : { cmd: "turnRight", btnName: "joy-right-btn"},
    JOY_BACK : { cmd: "backward", btnName: "joy-back-btn"},
}

InputEnum = {
    DISTANCE : "distance-input",
    SPEED : "speed-input",
    ROTATION:  "rotation-input",
    UNIT : "unit-input",
    CAL_LEFT_WHEEL : "left-wheel-input",
    CAL_RIGHT_WHEEL : "right-wheel-input",
    CAL_TURNING : "turning-cal-input",
    CAL_FRICTION : "friction-input",
    DIST_FRONT : "dist-front-output",
    GYRO_READ : "gyro-read-output",
    EVENTS : "event-area",
}

$(document).ready(function() {
    spark.login({accessToken: accessToken});
    spark.getEventStream(false, deviceID, function(data) {
        switch (data.name) {
            case EventEnum.COMPLETE:
                changeButtons(false);
                outputEvent(EventEnum.COMPLETE);
                break;
            case EventEnum.STOPPED:
                changeButtons(false);
                outputEvent(EventEnum.STOPPED);
                break;
            case EventEnum.FAILED:
                wasFail = true;
                changeButtons(true);
                outputEvent(EventEnum.FAILED);
                break;
            case EventEnum.HAS_FAILED:
                wasFail = true;
                changeButtons(true);
                outputEvent(EventEnum.HAS_FAILED);
                break;
            case EventEnum.CALIBRATION_VALUES:
                var array = base64js.toByteArray(data.data);
                outputCalibration(array);
                break;
            case EventEnum.ULTRASONIC_VALUES:
                var array = base64js.toByteArray(data.data);
                outputDistances(array);
                break;
            case EventEnum.GYROSCOPE_READINGS:
                var array = base64js.toByteArray(data.data);
                outputGyroReadings(array);
                break;
        }
    });

    setTimeout(getCalibrationValues, 1000);
    
    for (var btnEnumStr in ButtonEnum) {
        var button = ButtonEnum[btnEnumStr];
        $("#" + button.btnName).click(function(event) {
            var name = event.target.id;
            
            if (name == ButtonEnum.SEND_SPEED.btnName && 
                getInput(InputEnum.SPEED) != null) {
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
            
            if (name == ButtonEnum.CAL_FRICTION.btnName){
                particleCall(getCmd(name), getInput(InputEnum.CAL_FRICTION));
            }
            
            if (name == ButtonEnum.STOP.btnName) {
                particleCall(getCmd(name));
                changeButtons(false);
            }
            
            if (name == ButtonEnum.RESET_FAIL.btnName) {
                particleCall(getCmd(name));
                changeButtons(false);
            }
        });
    }
    
    for (var joyEnumStr in JoystickEnum) {
        var button = JoystickEnum[joyEnumStr];
        $("#" + button.btnName).mousedown(function() {
            var name = event.target.id;
            particleCall(getCmd(name));
            waitForLift = true;
        });
    }

    $(document).mouseup(function() {
        if (waitForLift) {
            particleCall("stop");
            waitForLift = false;
        }
    });
});

function outputEvent(eventName) {
    var date = new Date();
    var time = date.getTime();
    $("#" + InputEnum.EVENTS).append(eventName + " event at:" + time.toString() + "\n");
}

function outputCalibration(array) {
    setInput(InputEnum.SPEED, array[0] | (array[1] << 8));
    setInput(InputEnum.CAL_RIGHT_WHEEL, array[2] | (array[3] << 8));
    setInput(InputEnum.CAL_LEFT_WHEEL, array[4] | (array[5] << 8));
    setInput(InputEnum.CAL_TURNING, array[6] | (array[7] << 8));
    setInput(InputEnum.CAL_FRICTION, array[8] | (array[9] << 8));
}

function outputDistances(array){
    var value = array[0] | (array[1] << 8);
    setInput(InputEnum.DIST_FRONT, value.toString() + "cm");
}

function outputGyroReadings(array) {
    var i = 0;
    var ax = (array[i++] | (array[i++] << 8));
    if (ax > maxGyroRead) ax -= 0x10000;
    var ay = (array[i++] | (array[i++] << 8));
    if (ay > maxGyroRead) ay -= 0x10000;
    var az = (array[i++] | (array[i++] << 8));
    if (az > maxGyroRead) az -= 0x10000;
    var gx = (array[i++] | (array[i++] << 8));
    if (gx > maxGyroRead) gx -= 0x10000;
    var gy = (array[i++] | (array[i++] << 8));
    if (gy > maxGyroRead) gy -= 0x10000;
    var gz = (array[i++] | (array[i++] << 8));
    if (gz > maxGyroRead) gz -= 0x10000;
    
    var output = "ax: " + ax.toString() + ", ay: " + ay.toString() + ", az: " + az.toString() +
                        ", gx: " + gx.toString() + ", gy: " + gy.toString() + ", gz: " + gz.toString();
    setInput(InputEnum.GYRO_READ, output);
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
    for (var joyEnumStr in JoystickEnum){
        var button = JoystickEnum[joyEnumStr];
        if (button.btnName == btnName){
            return button.cmd;
        }
    }
    return null;
}

function changeButtons(disabled) {
    for (var btnEnumStr in ButtonEnum) {
        var button = ButtonEnum[btnEnumStr];
        if (disabled && ((!wasFail && button.btnName == ButtonEnum.STOP.btnName) ||
            (wasFail && button.btnName == ButtonEnum.RESET_FAIL.btnName))){
            continue;
        }
        
        $("#" + button.btnName).prop('disabled', disabled);
    }
    
    for (var joyEnumStr in JoystickEnum){
        var button = JoystickEnum[joyEnumStr];
        $("#" + button.btnName).prop('disabled', disabled);
    }
    wasFail = false;
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