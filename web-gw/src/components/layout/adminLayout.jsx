//import { ComponentChildren  } from "preact";

export default function AdminLayout({children}) {
    return (
        <main className="container" >
            {children}
        </main>
    );
}