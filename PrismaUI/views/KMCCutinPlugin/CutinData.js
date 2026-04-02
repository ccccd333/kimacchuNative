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
    }
}