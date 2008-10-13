// This plugin is like airspawn.js, but each user can set their own preference
// for how high to spawn using the /spawnheight command.

// Yes, this is pretty stupid, but it works ok as an example :)

slash_commands.spawnheight = function(height) {
    if (typeof height == "undefined") {
        // Give an example if the player didn't give a height.
        this.from.sendMessage("Example: /airspawn 20");
        return;
    }
    if (isNaN(height)) {   // Check to make sure the argument is a number.
        this.from.sendMessage("'"+height+"' isn't a number!");
        return;
    }

    this.from.spawn_height = Number(height);
    this.from.sendMessage("Your spawn height is "+height);
}

events.getPlayerSpawnPos.add(function () {
    if (this.player.spawn_height)
        this.pos[2] += this.player.spawn_height;
});

events.playerJoin.add(function (){
    this.player.sendMessage("Use the /spawnheight <height> command to spawn in the air!!!~!");
});

