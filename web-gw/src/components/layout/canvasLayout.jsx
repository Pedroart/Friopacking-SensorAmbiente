//import { ComponentChildren } from "preact";

export default function CanvasLayout({children}) {
    return (
        <main style={{ 
            display: 'flex',
            justifyContent: 'center',
            alignItems: 'center',
            height: '100vh'
        }} className="container" >
            {children}
        </main>
    );
}