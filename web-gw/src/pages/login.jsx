//import {useRef, useEffect} from "preact"

export default function Login() {
    return (
        <div style={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
        }}>
            <h1>Login</h1>
            <form>
                <input
                type="text"
                name="login"
                placeholder="Login"
                aria-label="Login"
                autoComplete="username"
                required
                />
                <input
                type="password"
                name="password"
                placeholder="Password"
                aria-label="Password"
                autoComplete="current-password"
                required
                />
                <fieldset>
                <label htmlFor="remember">
                    <input type="checkbox" role="switch" id="remember" name="remember" />
                    Remember me
                </label>
                </fieldset>
                <button type="submit">
                Login
                </button>
            </form>
        </div>
    )
}