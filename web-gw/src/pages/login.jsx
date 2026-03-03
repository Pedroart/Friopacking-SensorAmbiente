import "./login.css"

export default function Login() {
    return (
        <main className="login-main">
            <article className="login-card">
                <hgroup>
                    <h1>Bienvenido</h1>
                    <p>Por favor, ingrese sus credenciales.</p>
                </hgroup>
                <form>
                    <input
                        type="text"
                        name="login"
                        placeholder="Usuario"
                        aria-label="Usuario"
                        autoComplete="username"
                        required
                    />
                    <input
                        type="password"
                        name="password"
                        placeholder="Contraseña"
                        aria-label="Contraseña"
                        autoComplete="current-password"
                        required
                    />
                    <fieldset>
                        <label htmlFor="remember">
                            <input type="checkbox" role="switch" id="remember" name="remember" />
                            Recordarme
                        </label>
                    </fieldset>
                    <button type="submit">
                        Iniciar Sesión
                    </button>
                </form>
            </article>
        </main>
    )
}