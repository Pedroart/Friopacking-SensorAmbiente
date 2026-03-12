import { route } from 'preact-router';
import { useState } from 'preact/hooks';
import "./login.css";

export default function Login() {
    const [form, setForm] = useState({
        username: '',
        password: '',
        remember: false,
    });

    const [loading, setLoading] = useState(false);
    const [error, setError] = useState('');

    const handleChange = (e) => {
        const { name, value, type, checked } = e.target;
        setForm(prev => ({
            ...prev,
            [name]: type === 'checkbox' ? checked : value,
        }));
    };

    const handleSubmit = async (e) => {
        e.preventDefault();
        setLoading(true);
        setError('');

        try {
            // Mocked User Database from C++ struct:
            // Role 0: Viewer | Role 1: Operator | Role 2: Admin
            const users = [
                { username: 'viewer', password: 'viewer123', role: 0 },
                { username: 'operator', password: 'operator123', role: 1 },
                { username: 'admin', password: 'admin123', role: 2 },
            ];

            // Artificial delay to mimic network request
            await new Promise(r => setTimeout(r, 800));

            const userEntry = users.find(
                u => u.username === form.username && u.password === form.password
            );

            if (!userEntry) {
                throw new Error('Credenciales inválidas');
            }

            // Always use sessionStorage to force relogging on fresh tab/close
            sessionStorage.setItem('auth_user', userEntry.username);
            sessionStorage.setItem('auth_role', String(userEntry.role));

            // Optional: can use localStorage specifically ONLY for remember-me toggles 
            // but the auth token (in this case auth_user) stays session bound.
            if(form.remember) {
                localStorage.setItem('auth_remember_username', userEntry.username);
            } else {
                localStorage.removeItem('auth_remember_username');
            }

            route('/');
        } catch (err) {
            setError(err.message || 'Error al iniciar sesión');
        } finally {
            setLoading(false);
        }
    };

    return (
        <div className="login-main">
            <article className="login-card">
                <hgroup>
                    <h1>Bienvenido</h1>
                    <p>Por favor, ingrese sus credenciales.</p>
                </hgroup>

                <form onSubmit={handleSubmit}>
                    <input
                        type="text"
                        name="username"
                        placeholder="Usuario"
                        aria-label="Usuario"
                        autoComplete="username"
                        required
                        value={form.username}
                        onInput={handleChange}
                    />

                    <input
                        type="password"
                        name="password"
                        placeholder="Contraseña"
                        aria-label="Contraseña"
                        autoComplete="current-password"
                        required
                        value={form.password}
                        onInput={handleChange}
                    />

                    <fieldset>
                        <label htmlFor="remember">
                            <input
                                type="checkbox"
                                role="switch"
                                id="remember"
                                name="remember"
                                checked={form.remember}
                                onChange={handleChange}
                            />
                            Recordarme
                        </label>
                    </fieldset>

                    {error && <p className="login-error">{error}</p>}

                    <button type="submit" disabled={loading}>
                        {loading ? 'Ingresando...' : 'Iniciar Sesión'}
                    </button>
                </form>
            </article>
        </div>
    );
}