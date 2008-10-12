Object.create = function(o) {
    function F(){}
    F.prototype = o;
    return new F();
}

events = {};
events.create = function(event_name, player_attributes) {
    if (typeof player_attributes == "undefined")
        player_attributes = [];
    var o = Object.create(Event);
    o.event_name = event_name;
    //o.call_prototype = Object.create(Event.call_prototype)
    o.player_attributes = player_attributes;
    o._callbacks = [];
    if (typeof event_name != "undefined") {
        this[event_name] = o;
    }
    return o;
}

Event = {}
//Event.call_prototype = {}
Event.call = function(data) {
    var obj = Object.create(this);
    for (name in data)
        obj[name] = data[name];
    obj.preCall();
    for (var i=0; i<this._callbacks.length; i++) {
        this._callbacks[i].call(obj);
    }
    obj.postCall();
    for (name in data)
        data[name] = obj[name];
}

Event.preCall = function() {
    for (var i=0; i<this.player_attributes.length; i++) {
        var n = this.player_attributes[i];
        if (typeof this[n+"ID"] == "number") {
            this[n] = players.get(this[n+"ID"]);
        } else {
            this[n] = null;
        }
    }
}

Event.postCall = function(data) {
    for (var i=0; i<this.player_attributes.length; i++) {
        var n = this.player_attributes[i];
        if (this[n] && this[n].id) {
            this[n+"ID"] = this[n].id;
        } else {
            this[n+"ID"] = null;
        }
    }
}
 
Event.add = function(callback) {
    this._callbacks.push(callback);
}


events.create("getPlayerSpawnPos", ['player']);


players = [];
players._hash = {}

// Get (or create) a player object by it's id.
players.get = function(id) {
    if (typeof this._hash[id] == "undefined") {
        var p = new Player(id);
        this._hash[id] = p;
        this.push(p);
    }
    return this._hash[id];
}
players.remove = function(id) {
    this._hash[id] = undefined;
    for (var i=0; i<this.length; i++) {
        if (this[i].id == id) {
            this.splice(i, 1);
            break;
        }
    }
}


Player.prototype.sendMessage = function(message, from) {
    if (typeof from == "undefined")
        from = -2;
    sendTextMessage(from, this.id, message);
}



