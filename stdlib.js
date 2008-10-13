// Copyright 2008 by Matthew Marshall <matthew@matthewmarshall.org>
// License: GPL

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
    
    function _add_player_property(name) {
        o.__defineGetter__(name, function() {
            return players.get(this[name+"ID"]);
        });
        o.__defineSetter__(name, function(value) { // setter function is untested.
            if (typeof value == "undefined") {
                this[name+"ID"] = undefined;
            } else {
                this[name+"ID"] = value.id;
            }
        });
    }
    for (var i=0; i<player_attributes.length; i++)
        _add_player_property(player_attributes[i]);

    o._callbacks = [];
    if (typeof event_name != "undefined") {
        this[event_name] = o;
    }
    return o;
}

Event = {}
Event.call = function(data) {
    this._continue_propagation = true;
    var obj = Object.create(this);
    for (name in data)
        obj[name] = data[name];
    if (obj.preCall) obj.preCall();
    for (var i=0; i<this._callbacks.length; i++) {
        if (!obj._continue_propagation)
            break;
        this._callbacks[i].call(obj);
    }
    if (obj.postCall) obj.postCall();
    for (name in data)
        data[name] = obj[name];
}

Event.stopPropagation = function() {
    this._continue_propagation = false;
}

Event.add = function(callback) {
    this._callbacks.push(callback);
};


events.create("getPlayerSpawnPos", ['player']);
events.create("unknownSlashCommand", ['from']);
events.create("playerJoin", ['player']);
events.create("playerPart", ['player']);
events.create("playerDie", ['player', 'killer']);
events.create("tick");
events.create("flagGrabbed", ['player']);
events.create("flagDropped", ['player']);

events.playerPart.postCall = function() {
    players.remove(this.playerID);
};

slash_commands = {};

events.unknownSlashCommand.add(function(){
    if (this.handled) return;
    var obj = Object.create(this)
    obj.arguments = this.message.split(/\s+/g);
    obj.command = String(obj.arguments.splice(0,1)).replace("/", "");
    if (slash_commands[obj.command]) {
        this.handled = true;
        slash_commands[obj.command].apply(obj, obj.arguments);
    }
});
    

players = [];
players._hash = {}

// Get (or create) a player object by it's id.
players.get = function(id) {
    if (typeof id == "undefined")
        return undefined;
    if (typeof this._hash[id] == "undefined") {
        var p = new Player(id);
        this._hash[id] = p;
        this.push(p);
    }
    return this._hash[id];
};
players.remove = function(id) {
    this._hash[id] = undefined;
    for (var i=0; i<this.length; i++) {
        if (this[i].id == id) {
            this.splice(i, 1);
            break;
        }
    }
};


Player.prototype.sendMessage = function(message, from) {
    if (typeof from == "undefined")
        from = -2;
    _bz.sendTextMessage(from, this.id, message);
};

Player.prototype.kill = function(spawnOnBase, killer, flagType) {
    if (typeof killer != "number")
        killer = killer.id;
    return _bz.killPlayer(this.id, spawnOnBase, killer, flagType);
}

Player.prototype.giveFlag = function(flagType, force) {
    return _bz.givePlayerFlag(this.id, flagType, force);
}

Player.prototype.removeFlag = function(){
    return _bz.removePlayerFlag(this.id);
}

Player.prototype.__defineGetter__('team', function(){
    if (this.teamID == -1)
        return undefined;
    return teams[this.teamID];
});


function Team(id, name) {
    this.id = id;
    this.name = name;
}

Team.prototype.__defineGetter__('players',function(){
    return players.filter(function(player){return player.team == this}, this);
});

Team.prototype.sendMessage = function(message, from) {
    if (typeof from == "undefined")
        from = -2;
    _bz.sendTextMessageTeam(from, this.id, message);
};

teams = new (function(team_names){
    for (var i in team_names) {
        var name = team_names[i];
        this[name] = new Team(i, name);
        this[i] = this[name];
    }
})(['rogue', 'red', 'green', 'blue', 'purple', 'rabbit', 'hunter', 'observers', 'administrators']);

getCurrentTime = _bz.getCurrentTime;

