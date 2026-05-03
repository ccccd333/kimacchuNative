const TextureType = {
    CUTIN: "CUTIN",
    NAME: "NAME",
    BACKGROUND: "BACKGROUND"
};

export class Texture {
    constructor() {
        this.layers = new Map();
        this.single = false;
    }
}