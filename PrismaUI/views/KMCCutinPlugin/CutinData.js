const TextureType = {
    CUTIN: "CUTIN",
    NAME: "NAME",
    BACKGROUND: "BACKGROUND"
};

export class CutinData {
    constructor() {
        this.layers = new Map();
        this.single = false;
        this.display_time = 5.0;
        this.word = "";

        // "bold 20px sans-serif"
        this.actor_font_spec = ""; 
        this.actor_fill_color = "white";
        this.actor_shadow_color = "black";

        this.word_font_spec = "";
        this.word_fill_color = "white";
        this.word_shadow_color = "black";

        this.word_line_height = 24;
    }
}