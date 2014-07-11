function osciloscil() {

    var that = {};

    var numberOfWaves = 1;
    var buffersize = 1024;
    var waves;

    that.sine = (function(){
        var buffer = [];
        for ( var i = 0; i < buffersize; i++){
            buffer[i] = Math.sin(i / buffersize * 2 * Math.PI);
        }
        return buffer;
    })();

    that.square = (function(){
        var buffer = [];
        for ( var i = 0; i < buffersize; i++){
            buffer[i] = (i < buffersize / 2) ? 1 : -1;
        }
        return buffer;
    })();

    that.setwaves = function(wavestring){

        var buffer = [];

        if ( typeof wavestring === "string"  ){
            numberOfWaves = 1;

            buffer = that[wavestring]; 
            
            waves = wavestring;

            return buffer;
        }

        if ( wavestring instanceof Array){
            numberOfWaves = wavestring.length;

            for ( var i = 0; i < wavestring.length; i++){
                var temp = that[wavestring[i]];
                buffer = buffer.concat(temp);
            }

            waves = wavestring;

            return buffer;
        }
    }

    that.setfreq = function(f){
        that.synth.set("oscosc.freq", f/numberOfWaves);
    }

    that.synth = flock.synth({
        synthDef: {
            id : "oscosc",
        ugen : "flock.ugen.osc",
        freq : 100,
        table : that.setwaves(['sine','square']),
        }
    });

    return that;
}

