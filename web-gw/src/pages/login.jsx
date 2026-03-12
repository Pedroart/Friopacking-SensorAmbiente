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
            const response = await fetch('/api/login', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    username: form.username,
                    password: form.password,
                }),
            });

            const data = await response.json();

            if (!response.ok || !data.ok) {
                throw new Error(data.message || 'Credenciales inválidas');
            }

            // Guardar datos del usuario
            localStorage.setItem('auth_user', data.user);
            localStorage.setItem('auth_role', String(data.role));

            // opcional: guardar flag de recordarme
            localStorage.setItem('auth_remember', String(form.remember));

            route('/');
        } catch (err) {
            setError(err.message || 'Error al iniciar sesión');
        } finally {
            setLoading(false);
        }
    };

    return (
        <main className="login-main">
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
        </main>
    );
}