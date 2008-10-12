
Object.create = function(o) {
    function F(){}
    F.prototype = o;
    return new F();
}


events = {};
events.create = function(event_name) {
    var o = Object.create(Event);
    o.event_name = event_name;
    o._callbacks = [];
    if (typeof event_name != "undefined") {
        this[event_name] = o;
    }
    return o;
}

Event = {}
Event.call = function(data) {
    this.prepareData(data);
    for (var i=0; i<this._callbacks.length; i++) {
        this._callbacks[i](data, this);
    }
}
Event.prepareData = function(data) {
    if (typeof data.playerID == "number") {
        data.player = players.get(data.playerID);
    }
}

Event.add = function(callback) {
    this._callbacks.push(callback);
}


events.create("getPlayerSpawnPos");


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


