// src/WasmCanvas.js
import React, { useEffect } from 'react';

function WasmCanvas() {
  useEffect(() => {
    // Create a script element to load the Emscripten-generated JavaScript glue code
    const script = document.createElement("script");
    script.src = process.env.PUBLIC_URL + "/cube.js";
    script.async = true;
    document.body.appendChild(script);

    // Cleanup: remove the script when the component unmounts
    return () => {
      document.body.removeChild(script);
    };
  }, []);

  return (
    <div>
      <h2>Wasm Graphics Playground</h2>
      <p>This canvas is powered by a C++ WebAssembly module:</p>
      {/* The canvas element is already in index.html */}
    </div>
  );
}

export default WasmCanvas;
