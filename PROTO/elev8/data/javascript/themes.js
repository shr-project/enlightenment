var elm = require('elm');

var EXPAND_BOTH = { x: 1.0, y: 1.0 };
var FILL_BOTH = { x: -1.0, y: -1.0 };

var win = elm.realise(elm.Window({
    width: 320,
    height: 480,
    elements: {
        background: elm.Background({
            weight: EXPAND_BOTH,
            resize: true
        }),
        box: elm.Box({
            weight: EXPAND_BOTH,
            resize: true,
            elements: {
                overlay: elm.Button({
                    label: "Overlay",
                    icon: elm.Icon({ image: 'refresh' }),
                    on_click: function() {
                        elm.addThemeOverlay('./themes.edj');
                    }
                }),
                extension: elm.Button({
                    style: 'chucknorris',
                    label: 'Extension',
                    icon: elm.Icon({ image: 'apps' }),
                    on_click: function() {
                        elm.addThemeExtension('./themes.edj');
                    }
                }),
                remove: elm.Button({
                    style: 'chucknorris',
                    label: 'Delete',
                    icon: elm.Icon({ image: 'delete' }),
                    on_click: function() {
                        elm.delThemeExtension('./themes.edj');
                        elm.delThemeOverlay('./themes.edj');
                    }
                })
            }
        })
    }
}));
