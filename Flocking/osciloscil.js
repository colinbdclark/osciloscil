function osciloscil() {

    var that = {};

    var numberOfWaves = 1;
    var bufferSize = 1024;
    var waves;

    /* Arguments can be a string, list of strings, or an array of strings */
    that.setWaves = function (waves) {
        waves = fluid.makeArray(waves);
        var buffer = new Float32Array(bufferSize * waves.length);

        for (var i = 0; i < waves.length; i++){
            var temp = flock.fillTable(bufferSize, flock.tableGenerators[waves[i]]);
            buffer.set(temp, bufferSize * i);
        }

        return buffer;
    };

    that.setFreq = function(f){
        that.synth.set("oscosc.freq", f/numberOfWaves);
    };

    that.synth = flock.synth({
        synthDef: {
            id : "oscosc",
            ugen : "flock.ugen.osc",
            freq : 100,
            table : that.setWaves(['sin','square']),
        }
    });

    return that;
};

