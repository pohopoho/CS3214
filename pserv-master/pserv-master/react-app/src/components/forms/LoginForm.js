import React from 'react';
import { Row, Label, Input, Alert, Button, ButtonToolbar, Form, FormGroup } from 'reactstrap';
import { withFormik } from 'formik';

/*
 * This is a connected form that display username/password.
 * When the user hits submit, the 'onSubmit' method is called
 * in the parent, which receives the username/password the user
 * entered.  It also performs validation.
 */
class LoginForm extends React.Component {
  render() {
    const {
      handleSubmit, // rest is from HOC
      isSubmitting,
      handleReset,
      handleBlur,
      handleChange,
      // errors,
      dirty,
      // touched,
      values,
      // valid
    } = this.props;

    return (
      <Form onSubmit={handleSubmit}>
         <Row>
            <FormGroup>
              <Label for="username">User Name</Label>
              <Input type="text" name="username" value={values.username}
                onChange={handleChange}
                onBlur={handleBlur}
              />
            </FormGroup>
            <FormGroup>
              <Label for="password">Password</Label>
              <Input type="password" name="password"
                onChange={handleChange}
                onBlur={handleBlur}
              />
            </FormGroup>

         { this.props.autherror && 
             <FormGroup>
                <Alert bsstyle='danger'>
                  {this.props.autherror.message || 'Login failed'}
                </Alert>
             </FormGroup>
         }
         </Row>

         <Row>
             <ButtonToolbar>
                <Button
                  type='submit'
                  bsstyle='success' className="mr-2">
                  Submit
                </Button> 
                <Button
                  type="button"
                  onClick={handleReset}
                  disabled={!dirty || isSubmitting}>
                  Reset
                </Button>
             </ButtonToolbar>
         </Row>
      </Form>
    );
  }
}

/*
function mapStateToProps(state) {
  return {
    autherror: state.auth.error
  };
}
*/

export default withFormik({
  mapPropsToValues: () => ({ username: '', password: '' }),
  handleSubmit: (values, { setSubmitting, props }) => {
    props.onSubmit(values);
    setSubmitting(false);
  },
  displayName: 'LoginForm', // helps with React DevTools
})(LoginForm);

