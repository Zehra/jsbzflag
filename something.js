events.getPlayerSpawnPos.add(function () {
    if (this.player.spawn_height)
        this.pos[2] += this.player.spawn_height;
});

slash_commands.spawnheight = function(height) {
    if (isNaN(height)) {
        this.from.sendMessage("'"+height+"' isn't a number!");
        return;
    }
    this.from.spawn_height = Number(height);
    this.from.sendMessage("Set your spawn height to "+height);
}

events.playerJoin.add(function (){
    this.player.sendMessage("hey there!");
});
