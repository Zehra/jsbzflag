
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
    for (var i=0; i<this._callbacks.length; i++) {
        this._callbacks[i](data, this);
    }
}

Event.add = function(callback) {
    this._callbacks.push(callback);
}


events.create("getPlayerSpawnPos");


players = [];
