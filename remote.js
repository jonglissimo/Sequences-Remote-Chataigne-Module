var playTrigger = local.parameters.play;
var pauseTrigger = local.parameters.pause;
var togglePlayTrigger = local.parameters.togglePlay;
var stopTrigger = local.parameters.stop;
var stopAllTrigger = local.parameters.stopAll;
var selectedIndexParameter = local.parameters.selectedSequenceIndex;
var selectedNameParameter = local.parameters.selectedSequenceName;
var detectTrigger = local.parameters.detectRemotes;

var remotePort = 12033;
var localPort = 12034;

function init() {
	script.setUpdateRate(4);

	local.parameters.oscInput.localPort.setAttribute("readonly",true);
}

function update(deltaTime) {
	sendStatus();
}

function moduleParameterChanged(param){    
	if (param.is(playTrigger)) {
		play();
	} else if (param.is(pauseTrigger)) {
		pause();
	} else if (param.is(togglePlayTrigger)) {
		togglePlay();
	} else if (param.is(stopTrigger)) {
		stop();
	} else if (param.is(stopAllTrigger)) {
		stopAll();
	} else if (param.is(selectedIndexParameter)) {
		updateSelectedName();
	} else if (param.is(detectTrigger)) {
		detectRemotes();
	}
}

function oscEvent(address, args) {
	script.log("OSC Message received "+address+", "+args.length+" arguments");
	if (address == "/play") {
		playTrigger.trigger();
	} else if (address == "/pause") {
		pauseTrigger.trigger();
	} else if (address == "/togglePlay") {
		togglePlayTrigger.trigger();
	} else if (address == "/stop") {
		stopTrigger.trigger();
	} else if (address == "/stopAll") {
		stopAllTrigger.trigger();
	}  else if (address == "/next") {
		next();
	}  else if (address == "/prev") {
		prev();	
	}  else if (address == "/prevCue") {
		prevCue();
	} else if (address == "/wassup") {
		addRemote(args[0]);
	}
}

function play() {
	var sequence = getSelectedSequence();
	sequence.play.trigger();
}

function pause() {
	var sequence = getSelectedSequence();
	sequence.pause.trigger();
}

function stop() {
	var sequence = getSelectedSequence();
	sequence.stop.trigger();
}

function togglePlay() {
	var sequence = getSelectedSequence();
	sequence.togglePlay.trigger();
}

function stopAll() {
	root.sequences.stopAll.trigger();
}

function next() {
	var curIndex = selectedIndexParameter.get();
	var nextIndex = curIndex + 1;
	var lastIndex = root.sequences.getItems().length - 1;

	if (nextIndex <= lastIndex) {
		selectedIndexParameter.set(nextIndex);
	}
}

function prev() {
	var curIndex = selectedIndexParameter.get();
	var prevIndex = curIndex - 1;

	if (prevIndex >= 0) {
		selectedIndexParameter.set(prevIndex);
	}
}

function prevCue() {
	var sequence = getSelectedSequence();
	var curTime = sequence.currentTime.get();
	var cues = sequence.cues.getItems();
	var prevCueTime = 0;

	for (var i = 0; i < cues.length; i++) {
		var curCueTime = cues[i].time.get();

		if (curCueTime < curTime) {
			prevCueTime = curCueTime;
		}
	}

	sequence.currentTime.set(prevCueTime);
}

function getSelectedSequence() {
	var selectedIndex = selectedIndexParameter.get();
	var selectedSequence = root.sequences.getItemAt(selectedIndex);
	
	return selectedSequence;
}

function updateSelectedName() {
	var sequence = getSelectedSequence();
	var name = (sequence) ? sequence.niceName : "N/A";

	selectedNameParameter.set(name);
}

function detectRemotes() {
	var ips = util.getIPs();
	removeOutputs();
	
	for (var i = 0; i < ips.length; i++) {
		var ip = ips[i];
		var broadcastIP = getBroadcastIP(ip);
		script.log("Broadcast IP: " + broadcastIP);

		local.sendTo(broadcastIP, remotePort, "/yo", ip);
	}

}

//////////////////////////////////////////////////////
// OSC methods
//////////////////////////////////////////////////////

function sendStatus() {
	updateSelectedName();

	var index = selectedIndexParameter.get();
	var name = selectedNameParameter.get();
	var time = getTime();
	var isPlaying = getIsPlaying();

	local.send("/status", index, name, time, isPlaying);
}


//////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////

function getLastState(sequenceName) {
	for (var i = 0; i < sequenceStates.length; i++) {
		var cur = sequenceStates[i];
		if (cur.name == sequenceName) return cur;
	}
	
	return null;
}

function getTime() {
	var sequence = getSelectedSequence();
	var seconds = (sequence) ? sequence.currentTime.get() : 0;
	var m = parseInt(seconds / 60);
	var s = parseInt(seconds % 60);
	s = (s<10) ? "0" + s : s; 

	return "" + m + ":" + s;
}

function getIsPlaying() {
	var sequence = getSelectedSequence();
	var isPlaying = (sequence) ? sequence.isPlaying.get() : 0;
	return isPlaying;
}

function getBroadcastIP (ip) {
	digits = ip.split(".");
	return digits[0] + "." + digits[1] + "." + digits[2] + ".255";
}

function removeOutputs() {
	var outputs = local.parameters.oscOutputs;
	var items = outputs.getItems();

	script.log("outputs " + items.length);

	for (var i = 0; i < items.length; i++) {
		outputs.removeItem(items[i]);
	}
}

function addRemote(ip) {
	var outputs = local.parameters.oscOutputs;
	var newOutput = outputs.addItem();

	script.log("Add remote with IP: " +ip);
	newOutput.remoteHost.set(ip);
	newOutput.remotePort.set(remotePort);
	newOutput.local.set(false);
	newOutput.remotePort.setAttribute("readonly",true);
}

function getLastSequenceIndex() {
	var sequences = root.sequences.getItems().length;
} 